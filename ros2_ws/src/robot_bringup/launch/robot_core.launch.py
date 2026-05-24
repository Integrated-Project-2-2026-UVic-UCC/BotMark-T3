import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():

    # 1. Localizar la carpeta 'share' del paquete del LiDAR
    ldlidar_share_dir = get_package_share_directory("ldlidar_stl_ros2")

    return LaunchDescription(
        [
            # El Puente Zenoh (Cerebro <-> Músculos)
            Node(
                package="zenoh_bridge",
                executable="zenoh_bridge_node",
                name="zenoh_bridge_node",
                output="screen",
            ),
            # El Driver del LiDAR (Llamando a su propio launch)
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(ldlidar_share_dir, "launch", "ld19.launch.py")
                )
            ),
            # 3. El Detector de Obstáculos (El Reflejo de Supervivencia)
            Node(
                package="obstacle_detector",
                executable="obstacle_detector_node",
                name="obstacle_detector_node",
                output="screen",
            ),
            # 4. El Controlador Cinemático (El Chef)
            Node(
                package="controller_pkg",
                executable="controller_node",
                name="controller_node",
                output="screen",
            ),
            # 5. El Mission Manager (El Capitán)
            Node(
                package="mission_manager",
                executable="mission_node",  # Cambia al nombre exacto de tu ejecutable
                name="mission_node",
                output="screen",
            ),
        ]
    )
