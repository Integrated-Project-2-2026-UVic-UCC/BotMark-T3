import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription, ExecuteProcess
from launch.launch_description_sources import PythonLaunchDescriptionSource

def generate_launch_description():

    # Localizar la carpeta 'share' del paquete del LiDAR
    ldlidar_share_dir = get_package_share_directory("ldlidar_stl_ros2")

    # Construir la ruta absoluta al ejecutable de Zenoh
    home_dir = os.path.expanduser('~')
    zenohd_path = os.path.join(home_dir, 'zenoh-1.8.0-x86_64-unknown-linux-gnu-standalone', 'zenohd')

    return LaunchDescription(
        [
            # El Router Zenoh (Ejecutable del sistema)
            ExecuteProcess(
                cmd=[zenohd_path, '-l', 'tcp/0.0.0.0:7447'],
                name='zenoh_router',
                output='screen',
                prefix='terminator -u -x '  # Importante: deja el espacio al final después de la 'x'
            ),

            # El Puente Zenoh (Cerebro <-> Músculos)
            Node(
                package="zenoh_bridge",
                executable="zenoh_bridge_node",
                name="zenoh_bridge_node",
                output="screen",
            ),
            ExecuteProcess(
                cmd=['ros2', 'launch', 'ldlidar_stl_ros2', 'ld19.launch.py'],
                output='screen',
                name='ldlidar_launch',
                prefix='terminator -u -x '  # <-- Abre otra ventana de Terminator
            ),
            # El Detector de Obstáculos (El Reflejo de Supervivencia)
            Node(
                package="obstacle_detector",
                executable="obstacle_detector_node",
                name="obstacle_detector_node",
                output="screen",
            ),
            # El Controlador Cinemático (El Chef)
            Node(
                package="controller_pkg",
                executable="controller_node",
                name="controller_node",
                output="screen",
            ),
            # El Mission Manager (El Capitán)
            Node(
                package="mission_manager",
                executable="mission_node",  # Cambia al nombre exacto de tu ejecutable
                name="mission_node",
                output="screen",
            ),
        ]
    )

