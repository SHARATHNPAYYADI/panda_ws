import os
import sys
import time
import unittest

import launch
import launch_ros
import launch_testing.actions
import rclpy

from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import TimerAction, IncludeLaunchDescription
import os

from panda_msgs.srv import MoveToGoal

from ament_index_python.packages import get_package_share_directory

def generate_test_description():
    bringup_dir = get_package_share_directory('panda_bringup')
    launch_file = os.path.join(bringup_dir, 'launch', 'camera_based_pick_and_place.launch.py')

    return (
        launch.LaunchDescription(
            [
                # Nodes under test
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource(launch_file)
                ),
                # Launch tests 0.5 s later
                launch.actions.TimerAction(
                    period=10.0, actions=[launch_testing.actions.ReadyToTest()]),
            ]
        ), {},
    )

# Active tests
class TestPandaSim(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init()

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def setUp(self):
        self.node = rclpy.create_node('test_move_goal_sim')

    def tearDown(self):
        self.node.destroy_node()

    def test_moving_goal_valid(self, proc_output):
        """Check if the planner plans and executes the valid plan with given position value"""
        client = self.node.create_client(MoveToGoal, '/move_to_goal')

        # Wait for the service to exist
        timeout = time.time() + 15.0
        while not client.wait_for_service(timeout_sec=1.0):
            if time.time() > timeout:
                raise RuntimeError("Service /move_to_goal did not become available")
            self.node.get_logger().info("Waiting for /move_to_goal service...")

        # Prepare request
        req = MoveToGoal.Request()
        req.x = 0.64
        req.y = -0.2
        req.z = 0.2
        req.orie_x = 0.0
        req.orie_y = 0.0
        req.orie_z = 0.0
        req.orie_w = 1.0

        # Send request
        future = client.call_async(req)

        # Spin until we get a response (max 5s)

        end_time = time.time() + 20.0
        while rclpy.ok() and not future.done() and time.time() < end_time:
            rclpy.spin_once(self.node, timeout_sec=0.1)
        
        assert future.done(), "Service call to /move_to_goal timed out"
        response = future.result()
        print("Full response:", response)
        # **Check that the service correctly passed**
        assert response.success, "move_to_goal service passed with valid position"

# Post-shutdown tests
@launch_testing.post_shutdown_test()
class TestPandaSimShutdown(unittest.TestCase):
    def test_exit_codes(self, proc_info):
        """Check if the processes exited normally or were killed on shutdown."""
        launch_testing.asserts.assertExitCodes(
            proc_info,
            allowable_exit_codes=[0, -9, -11, -6, -2]  
        )
