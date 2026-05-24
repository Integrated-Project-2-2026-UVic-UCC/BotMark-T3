import rclpy, os, sys, yaml, math

from rclpy.node import Node
from rclpy.action import ActionClient
from nav2_msgs.action import NavigateToPose
from ament_index_python.packages import get_package_share_directory


def euler_to_quaternion(yaw_degrees):
    yaw_radians = math.radians(yaw_degrees)
    """Convierte el ángulo Theta (Z) a un Cuaternión que ROS 2 entienda."""
    qx = 0.0
    qy = 0.0
    qz = math.sin(yaw_radians / 2.0)
    qw = math.cos(yaw_radians / 2.0)
    return qx, qy, qz, qw


class MissionManagerNode(Node):
    def __init__(self):
        super().__init__("mission_manager")

        # 1. Crear el Action Client
        self._action_client = ActionClient(self, NavigateToPose, "navigate_to_pose")

        # 2. Cargar el YAML
        self.waypoints = []
        self.current_wp_index = 0
        self.load_mission()

        # 3. Arrancar la misión tras 2 segundos de inicialización (dar tiempo a que el nodo se levante y el Action Client se conecte al servidor)
        self.get_logger().info("Esperando conexión con el Controlador...")
        self.timer = self.create_timer(2.0, self.start_mission)
        # El timer se ejecutará una sola vez para iniciar la misión
        # Se hace con timer en vez de time.sleep porque asi no bloqueamos el hilo principal y el nodo escucha quien se suscribe al topic de feedback del Action Client desde el inicio.

    def load_mission(self):
        try:
            package_share_directory = get_package_share_directory("mission_manager")
            yaml_path = os.path.join(
                package_share_directory, "config", "test_route.yaml"
            )

            with open(yaml_path, "r") as file:
                data = yaml.safe_load(file)
                self.waypoints = data.get("waypoints", [])
                self.get_logger().info(f"Misión cargada: {len(self.waypoints)} puntos.")
        except Exception as e:
            self.get_logger().error(f"Fallo al leer YAML: {e}")
            sys.exit(1)

    def start_mission(self):
        self.timer.cancel()  # Cancelamos el timer para que no se repita (single-shot)
        self._action_client.wait_for_server()
        self.send_next_waypoint()

    def send_next_waypoint(self):
        """Prepara y envía el siguiente punto al Controlador."""
        if self.current_wp_index >= len(self.waypoints):
            self.get_logger().info("✅ ¡MISIÓN COMPLETADA CON ÉXITO!")
            rclpy.shutdown()
            return

        wp = self.waypoints[self.current_wp_index]
        self.get_logger().info(
            f"🚀 Viajando al Punto {wp['id']} -> X: {wp['x']}, Y: {wp['y']}"
        )

        # Crear el mensaje Goal estándar de Nav2
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

        # Action es como servidor pero con la capacidad de enviar feedback continuo y recibir el resultado final
        # Es como un servidor que en medio tiene un publish/subscribe
        send_goal_future = self._action_client.send_goal_async(
            goal_msg, feedback_callback=self.feedback_callback
        )  # envia objectiu i asigna que fer al rebre feedback, el feedback no se ejecua hasta que el primer callback (goal_response_callback) diga que el objetivo fue aceptado por el controlador
        send_goal_future.add_done_callback(
            self.goal_response_callback
        )  # Posa alarma fins que el send_goal_future sigui acceptat o rebutjat pel controlador, i llavors executa el callback goal_response_callback

    def feedback_callback(self, feedback_msg):
        """Se ejecuta continuamente mientras el robot se mueve (Feedback)"""
        distancia = feedback_msg.feedback.distance_remaining
        self.get_logger().info(
            f"Distancia al objetivo: {distancia:.2f} metros", throttle_duration_sec=2.0
        )
        # throttle_duration_sec hace que el mensaje se imprima cada 2 segundos, evitando saturar la consola con demasiados mensajes de feedback
        # ignora en silencio todos los prints que pasen por esta misma línea de código durante los próximos 2 segundos

    def goal_response_callback(self, future):
        """Verifica si el Controlador aceptó o rechazó la orden"""
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.get_logger().error(
                "❌ El Controlador rechazó el punto. Abortando misión."
            )
            return

        self.get_logger().info("Punto aceptado por el Controlador. En movimiento...")
        get_result_future = goal_handle.get_result_async()
        get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        """Se ejecuta cuando el Controlador dice 'Llegué' o 'Fallé'"""
        status = future.result().status
        if status == 4:  # SUCCEEDED
            self.get_logger().info(
                f"🏁 Punto {self.waypoints[self.current_wp_index]['id']} alcanzado."
            )
            self.current_wp_index += 1
            self.send_next_waypoint()  # Pasar al siguiente
        else:
            self.get_logger().error(
                f"⚠️ El robot falló al llegar al punto. Código de estado: {status}"
            )


def main(args=None):
    rclpy.init(args=args)
    node = MissionManagerNode()
    try:
        rclpy.spin(node)  # spin() mantiene los callbacks asíncronos vivos
    except KeyboardInterrupt:
        node.get_logger().info("Misión cancelada por el usuario.")
    finally:
        node.destroy_node()


if __name__ == "__main__":
    main()
