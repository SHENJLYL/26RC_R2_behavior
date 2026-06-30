cmake_minimum_required(VERSION 3.16)
project(r2_behavior)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(behaviortree_cpp_v3 REQUIRED)
find_package(Curses REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

include_directories(include)

add_executable(
  r2_behavior_server
  src/r2_behavior_server.cpp
  plugins/condition/is_manual_start.cpp
  plugins/action/pub_nav2_goal.cpp
  plugins/action/pub_twist.cpp
)
ament_target_dependencies(
  r2_behavior_server
  ament_index_cpp
  behaviortree_cpp_v3
  geometry_msgs
  rclcpp
  std_msgs
  tf2_geometry_msgs
)
target_link_libraries(r2_behavior_server ${CURSES_LIBRARIES} tinfo)

add_executable(r2_behavior_client src/r2_behavior_client.cpp)
ament_target_dependencies(
  r2_behavior_client
  rclcpp
  std_msgs
)

install(
  TARGETS
    r2_behavior_server
    r2_behavior_client
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY behavior_trees DESTINATION share/${PROJECT_NAME})
install(DIRECTORY docs DESTINATION share/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY params DESTINATION share/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

ament_package()
