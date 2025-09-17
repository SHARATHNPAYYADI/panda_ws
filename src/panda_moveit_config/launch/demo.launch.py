from moveit_configs_utils import MoveItConfigsBuilder
from moveit_configs_utils.launches import generate_demo_launch
from launch import LaunchDescription
from launch_ros.actions import SetParameter

def generate_launch_description():
    moveit_config = MoveItConfigsBuilder("panda", package_name="panda_moveit_config").to_moveit_configs()
    return LaunchDescription([
        # Force all nodes to use /clock from Gazebo
        SetParameter(name="use_sim_time", value=True),

        # Standard MoveIt demo launch
        generate_demo_launch(moveit_config),
    ])
    # return generate_demo_launch(moveit_config)
