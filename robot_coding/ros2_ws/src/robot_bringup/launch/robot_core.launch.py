"""
This script initializes the complete autonomous robot software stack, including the Zenoh router 
for network communication, hardware drivers, safety nodes, and autonomous navigation controllers.
"""

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, SetEnvironmentVariable

def generate_launch_description():
    # Dynamically search the system for the zenohd executable
    current_file_dir = os.path.dirname(os.path.realpath(__file__))

    zenohd_path = os.path.join(current_file_dir, '../../../../zenoh_router_v1.8.0/zenohd')

    return LaunchDescription(
        [
            # Force colors in ROS 2 logs
            SetEnvironmentVariable('RCUTILS_COLORIZED_OUTPUT', '1'),
            SetEnvironmentVariable('RCUTILS_CONSOLE_OUTPUT_FORMAT', '[{severity}] [{name}]: {message}'),

            # Zenoh Router Process
            # Establishes the core communication network on a specific TCP port
            ExecuteProcess(
                cmd=[zenohd_path, '-l', 'tcp/0.0.0.0:7447'],
                name='zenoh_router',
                output='screen',
                prefix='terminator -u -x '  # Required trailing space to execute command in a new terminal window
            ),

            # Zenoh Bridge Node
            # Bridges data streams between the ROS 2 ecosystem and the microcontrollers via Zenoh
            Node(
                package="zenoh_bridge",
                executable="zenoh_bridge_node",
                name="zenoh_bridge_node",
                output="screen",
                respawn=True,
                respawn_delay=2.0,
            ),
            
            # LiDAR Driver Launch Process
            # Executes the manufacturer-provided launch file for the LiDAR sensor
            ExecuteProcess(
                cmd=['ros2', 'launch', 'ldlidar_stl_ros2', 'ld19.launch.py'],
                output='screen',
                name='ldlidar_launch',
                prefix='terminator -u -x '  # Opens execution in a separate Terminator window
            ),
            
            # Obstacle Detector Node
            # Processes LiDAR point clouds dynamically to trigger emergency safety stops
            Node(
                package="obstacle_detector",
                executable="obstacle_detector_node",
                name="obstacle_detector_node",
                output="screen",
                respawn=True,
                respawn_delay=2.0,
            ),
            
            # Kinematic Controller Node
            # Manages odometry calculations and executes twist commands into wheel velocities
            Node(
                package="controller_pkg",
                executable="controller_node",
                name="controller_node",
                output="screen",
                respawn=True,
                respawn_delay=2.0,
            ),
            
            # Mission Manager Node
            # Handles high-level navigation sequencing and waypoint management
            Node(
                package="mission_manager",
                executable="mission_node",  
                name="mission_node",
                output="screen",
                respawn=True,
                respawn_delay=2.0,
            ),
        ]
    )