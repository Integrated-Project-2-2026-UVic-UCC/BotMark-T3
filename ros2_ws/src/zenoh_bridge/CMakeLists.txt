cmake_minimum_required(VERSION 3.8)
project(zenoh_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

# Buscar la librería C de Zenoh instalada en Ubuntu (zenoh-c)
find_library(ZENOHC_LIBRARY NAMES zenohc REQUIRED)
find_path(ZENOHC_INCLUDE_DIR NAMES zenoh.h REQUIRED)

include_directories(
  include
  ${ZENOHC_INCLUDE_DIR}
)

# Crear el ejecutable desde tu archivo .cpp
add_executable(zenoh_bridge_node src/zenoh_bridge_node.cpp)

# Enlazar la librería C de Zenoh al ejecutable
target_link_libraries(zenoh_bridge_node ${ZENOHC_LIBRARY})

# Enlazar las dependencias de ROS 2 al ejecutable
ament_target_dependencies(
  zenoh_bridge_node
  rclcpp
  nav_msgs
  sensor_msgs
  geometry_msgs
  std_msgs
  tf2
  tf2_geometry_msgs
)

# Reglas de instalación para ROS 2
install(TARGETS zenoh_bridge_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY include/
  DESTINATION include)

ament_package()
