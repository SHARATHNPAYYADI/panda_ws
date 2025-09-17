cmake_minimum_required(VERSION 3.8)
project(panda_integration_test)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(panda_msgs REQUIRED)
# find_package(panda_bringup REQUIRED)
find_package(panda_moveit_config REQUIRED)
# find_package(panda_object_detection REQUIRED)
find_package(panda_motion_planning REQUIRED)

# ament_python_install_package(${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_ros REQUIRED)
  find_package(launch_testing_ament_cmake REQUIRED)
  function(add_ros_isolated_launch_test path)
    set(RUNNER "${ament_cmake_ros_DIR}/run_test_isolated.py")
    add_launch_test("${path}" RUNNER "${RUNNER}" ${ARGN})
  endfunction()
  add_ros_isolated_launch_test(test/test_integration.py)
  add_ros_isolated_launch_test(test/test_gripper.py)
  add_ros_isolated_launch_test(test/test_camera_detection.py)
  add_ros_isolated_launch_test(test/test_move_to_goal.py)
endif()

ament_package()


