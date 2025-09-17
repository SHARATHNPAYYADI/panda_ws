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

from panda_msgs.srv import MoveGripper
from panda_msgs.srv import MoveJoints

from ament_index_python.packages import get_package_share_directory

def generate_test_description():
    bringup_dir = get_package_share_directory('panda_bringup')
    launch_file = os.path.join(bringup_dir, 'launch', 'simple_pick_and_place.launch.py')

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
        self.node = rclpy.create_node('test_gripper_sim')

    def tearDown(self):
        self.node.destroy_node()

    def test_gripper_open(self, proc_output):
        """Check whether gripper works as intended published"""
        # try:
        # Create service client
        client = self.node.create_client(MoveGripper, '/move_gripper')

        # Wait for service to be available
        assert client.wait_for_service(timeout_sec=10.0), "Service /move_gripper not available"

        # Prepare request
        req = MoveGripper.Request()
        req.position = 0.04   # Example value

        # Send request
        future = client.call_async(req)

        # Spin until we get a response (max 5s)
        end_time = time.time() + 5.0
        while rclpy.ok() and not future.done() and time.time() < end_time:
            rclpy.spin_once(self.node, timeout_sec=0.1)

        assert future.done(), "Service call to /move_gripper timed out"
        response = future.result()
        assert response.success, "Service call failed"
    
    def test_gripper_invalid(self, proc_output):
        """Check whether gripper fails as expected for invalid values"""
        client = self.node.create_client(MoveGripper, '/move_gripper')

        # Wait for service to be available
        assert client.wait_for_service(timeout_sec=10.0), "Service /move_gripper not available"

        # Prepare request
        req = MoveGripper.Request()
        req.position = 0.5   # invalid value

        # Send request
        future = client.call_async(req)

        # Spin until we get a response (max 5s)
        end_time = time.time() + 5.0
        while rclpy.ok() and not future.done() and time.time() < end_time:
            rclpy.spin_once(self.node, timeout_sec=0.1)

        assert future.done(), "Service call to /move_gripper timed out"
        response = future.result()

        # **Check that the service correctly failed**
        assert not response.success, "Gripper moved with invalid input!"

    


# Post-shutdown tests
@launch_testing.post_shutdown_test()
class TestPandaSimShutdown(unittest.TestCase):
    def test_exit_codes(self, proc_info):
        """Check if the processes exited normally or were killed on shutdown."""
        launch_testing.asserts.assertExitCodes(
            proc_info,
            allowable_exit_codes=[0, -9, -11, -6, -2]  
        )
