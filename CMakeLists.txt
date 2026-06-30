cmake_minimum_required(VERSION 3.16)
project(r2_behavior)

find_package(ament_cmake REQUIRED)

install(DIRECTORY behavior_trees DESTINATION share/${PROJECT_NAME})
install(DIRECTORY docs DESTINATION share/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY params DESTINATION share/${PROJECT_NAME})

ament_package()
