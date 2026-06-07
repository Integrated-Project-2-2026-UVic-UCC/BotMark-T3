cmake_minimum_required(VERSION 3.8)
project(obstacle_detector)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

# 2. Crear el ejecutable
add_executable(obstacle_detector_node
  src/obstacle_detector_node.cpp
)

target_include_directories(obstacle_detector_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# 4. Vincular dependencias de ROS 2
ament_target_dependencies(
  obstacle_detector_node
  rclcpp
  std_msgs
  geometry_msgs
  sensor_msgs
)

# 5. Reglas de instalación
install(TARGETS
  obstacle_detector_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

ament_package()
