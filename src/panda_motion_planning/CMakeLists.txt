cmake_minimum_required(VERSION 3.8)
project(panda_motion_planning)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---------------------------
# Find dependencies
# ---------------------------
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(control_msgs REQUIRED)
find_package(panda_msgs REQUIRED)
find_package(linkattacher_msgs REQUIRED)

# ---------------------------
# Include directories
# ---------------------------
# All headers are in include/panda_motion_planning
include_directories(include)

# ---------------------------
# Executables
# ---------------------------
add_executable(panda_motion_planning src/panda_motion_planning.cpp)
add_executable(panda_pose_logger src/panda_pose_logger.cpp)
add_executable(panda_joint_goal src/panda_joint_goal.cpp)
add_executable(simple_pick_and_place src/simple_pick_and_place.cpp)
add_executable(camera_based_pick_and_place src/camera_based_pick_and_place.cpp)

# ---------------------------
# Link dependencies
# ---------------------------
ament_target_dependencies(panda_motion_planning
  rclcpp
  moveit_ros_planning_interface
  panda_msgs
  control_msgs
)

ament_target_dependencies(panda_pose_logger
  rclcpp
  moveit_ros_planning_interface
  panda_msgs
  control_msgs
)

ament_target_dependencies(panda_joint_goal
  rclcpp
  moveit_ros_planning_interface
  panda_msgs
  control_msgs
)

ament_target_dependencies(simple_pick_and_place
  rclcpp
  moveit_ros_planning_interface
  panda_msgs
  control_msgs
  linkattacher_msgs
)

ament_target_dependencies(camera_based_pick_and_place
  rclcpp
  moveit_ros_planning_interface
  panda_msgs
  control_msgs
  linkattacher_msgs
)

# ---------------------------
# Install targets
# ---------------------------
install(TARGETS
  panda_motion_planning
  panda_pose_logger
  panda_joint_goal
  simple_pick_and_place
  camera_based_pick_and_place
  DESTINATION lib/${PROJECT_NAME}
)

# ---------------------------
# Install headers
# ---------------------------
install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# ---------------------------
# Export includes for other packages
# ---------------------------
ament_export_include_directories(include)
ament_export_dependencies(
  rclcpp
  moveit_ros_planning_interface
  panda_msgs
  control_msgs
  linkattacher_msgs
)

# ---------------------------
# Testing
# ---------------------------
if(BUILD_TESTING)
  # find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(${PROJECT_NAME}_testing test/test_joint_services.cpp)
  # target_link_libraries(${PROJECT_NAME}_testing panda_motion_planning_lib)
  target_include_directories(${PROJECT_NAME}_testing PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  ament_target_dependencies(${PROJECT_NAME}_testing
    panda_msgs
    std_msgs
    rclcpp
    moveit_ros_planning_interface
  )
endif()

# ---------------------------
# Package
# ---------------------------
ament_package()
