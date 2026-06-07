"""
This node manages autonomous navigation by sequentially sending waypoints 
to the controller node via a ROS 2 Action Server. It monitors state changes 
such as emergency stops to pause or abort missions accordingly.
"""
import rclpy, os, sys, yaml, math

from rclpy.node import Node
from rclpy.action import ActionClient
from nav2_msgs.action import NavigateToPose
from ament_index_python.packages import get_package_share_directory
from std_msgs.msg import Bool


def euler_to_quaternion(yaw_degrees):
    # Converts a yaw angle (Z-axis rotation) from degrees to a quaternion representation.
    yaw_radians = math.radians(yaw_degrees)
    qx = 0.0
    qy = 0.0
    qz = math.sin(yaw_radians / 2.0)
    qw = math.cos(yaw_radians / 2.0)
    return qx, qy, qz, qw


class MissionManagerNode(Node):
    def __init__(self):
        super().__init__("mission_manager")

        # Initialize ROS 2 Action Client to interface with the controller node
        self._action_client = ActionClient(self, NavigateToPose, "navigate_to_pose")

        # State tracking variables for emergency stops
        self.is_stopped = True
        self.waiting_for_resume = False
        
        # Subscribe to the 'is_stoped' topic to monitor emergency stop commands
        self.stop_sub = self.create_subscription(
            Bool, 
            "is_stoped", 
            self.stop_callback, 
            10
        )

        # Initialize mission variables and load waypoints from configuration
        self.waypoints = []
        self.current_wp_index = 0
        self.load_mission()

        # Delay mission execution by 2 seconds to allow node initialization
        self.get_logger().info("Waiting for connection with Controller...")
        self.timer = self.create_timer(2.0, self.start_mission)

    def load_mission(self):
        # Loads sequence of target coordinates from a YAML configuration file.
        try:
            package_share_directory = get_package_share_directory("mission_manager")
            yaml_path = os.path.join(
                package_share_directory, "config", "test_route.yaml"
            )

            with open(yaml_path, "r") as file:
                data = yaml.safe_load(file)
                self.waypoints = data.get("waypoints", [])
                self.get_logger().info(f"Mission loaded: {len(self.waypoints)} points.")
        except Exception as e:
            self.get_logger().error(f"Failed to read YAML configuration: {e}")
            sys.exit(1)

    def stop_callback(self, msg):
        # Processes incoming emergency stop signals, aborting current tasks if active,
        # and re-initiating the sequence upon resumption.
        self.is_stopped = msg.data
        
        if self.is_stopped:
            self.get_logger().error("STOP SIGNAL RECEIVED! Mission aborted. Resetting to waypoint 1.")
            self.current_wp_index = 0
            self.waiting_for_resume = True
            # Note: Explicit cancellation not sent here as the controller node 
            # also subscribes to the stop topic and will self-abort the action.
        else:
            if self.waiting_for_resume:
                self.get_logger().info("STOP SIGNAL LIFTED. Resuming mission from the beginning.")
                self.waiting_for_resume = False
                self.send_next_waypoint()

    def start_mission(self):
        # Terminates the initialization delay timer and triggers the first waypoint.
        self.timer.cancel()
        self._action_client.wait_for_server()
        self.send_next_waypoint()

    def send_next_waypoint(self):
        # Prepares and transmits the next target coordinate in the sequence to the controller.
        
        # Halt execution if an emergency stop is active
        if self.is_stopped:
            self.get_logger().warn("Mission blocked by STOP state. Waiting for resumption...")
            return

        # Check if all waypoints have been successfully navigated
        if self.current_wp_index >= len(self.waypoints):
            self.get_logger().info("MISSION SUCCESSFULLY COMPLETED!")
            rclpy.shutdown()
            return

        wp = self.waypoints[self.current_wp_index]
        self.get_logger().info(
            f"Traveling to Point {wp['id']} -> X: {wp['x']}, Y: {wp['y']}"
        )

        # Construct standard Nav2 NavigateToPose action goal
        goal_msg = NavigateToPose.Goal()
        goal_msg.pose.header.frame_id = "map"
        goal_msg.pose.header.stamp = self.get_clock().now().to_msg()

        goal_msg.pose.pose.position.x = float(wp["x"])
        goal_msg.pose.pose.position.y = float(wp["y"])

        qx, qy, qz, qw = euler_to_quaternion(float(wp["theta"]))
        goal_msg.pose.pose.orientation.x = qx
        goal_msg.pose.pose.orientation.y = qy
        goal_msg.pose.pose.orientation.z = qz
        goal_msg.pose.pose.orientation.w = qw

        # Transmit goal asynchronously and assign callbacks
        send_goal_future = self._action_client.send_goal_async(
            goal_msg, feedback_callback=self.feedback_callback
        )
        send_goal_future.add_done_callback(self.goal_response_callback)

    def feedback_callback(self, feedback_msg):
        # Processes real-time feedback regarding distance to target during active navigation.
        distancia = feedback_msg.feedback.distance_remaining
        self.get_logger().info(
            f"Distance to target: {distancia:.2f} meters", throttle_duration_sec=2.0
        )

    def goal_response_callback(self, future):
        # Evaluates whether the controller node accepted or rejected the issued waypoint.
        goal_handle = future.result()
        if not goal_handle.accepted:
            # Rejection typically occurs if the controller receives a goal while in STOP mode
            self.get_logger().error("Controller rejected the waypoint.")
            return

        self.get_logger().info("Waypoint accepted by Controller. In motion...")
        get_result_future = goal_handle.get_result_async()
        get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        # Evaluates the final status of a navigation attempt (success, failure, or abort).
        status = future.result().status
        
        if status == 4:  # SUCCEEDED status code
            self.get_logger().info(
                f"Point {self.waypoints[self.current_wp_index]['id']} reached."
            )
            self.current_wp_index += 1
            self.send_next_waypoint()  
            
        elif status == 6:  # ABORTED status code
            if self.is_stopped:
                self.get_logger().warn("Current mission aborted correctly due to STOP signal.")
            else:
                self.get_logger().error("Controller aborted the mission unexpectedly.")
        
        else:
            self.get_logger().error(
                f"Robot failed to reach the point. Status code: {status}"
            )


def main(args=None):
    rclpy.init(args=args)
    node = MissionManagerNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("Mission canceled by user.")
    finally:
        node.destroy_node()


if __name__ == "__main__":
    main()