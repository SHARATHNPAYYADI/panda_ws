from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, ExecuteProcess
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # Paths
    pkg_share = get_package_share_directory('panda_bringup')
    urdf_path = os.path.join(
        get_package_share_directory('panda_bringup'),
        'description',
        'models',
        'panda',
        'panda.urdf.xacro'
    )

    empty_world = os.path.join(
    get_package_share_directory('gazebo_ros'),
    'worlds',
    'empty.world'
    )
    simple_world = os.path.join(
    get_package_share_directory('panda_bringup'),
    'worlds',
    'simple_world.world'
    )
    controllers_yaml = os.path.join(pkg_share, 'config', 'ros_control.yaml')

    # Launch description
    return LaunchDescription([
        # Robot State Publisher with xacro
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{
                'robot_description': Command(['xacro ', urdf_path])
            }]
        ),

        # Gazebo launch
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(get_package_share_directory('gazebo_ros'), 'launch', 'gazebo.launch.py')
            ),
            launch_arguments={'world': simple_world}.items()
        ),
        Node(
            package='controller_manager',
            executable='ros2_control_node',
            parameters=[
                controllers_yaml
            ],
            output='screen'
        ),
        # Spawn robot in Gazebo
        Node(
            package='gazebo_ros',
            executable='spawn_entity.py',
            arguments=['-topic', 'robot_description', '-entity', 'my_robot', '-reference_frame', 'world'],
            output='screen'
        ),
        Node(
            package='controller_manager',
            executable='spawner',
            arguments=["joint_state_broadcaster"],
            output='screen'
        ),
        # spawn arm controller
        Node(
            package='controller_manager',
            executable='spawner',
            arguments=["panda_arm_controller"],
            output='screen'
        ),
        Node(
            package='controller_manager',
            executable='spawner',
            arguments=["panda_gripper_controller_right"],
            output='screen'
        ),
        Node(
            package='controller_manager',
            executable='spawner',
            arguments=["panda_gripper_controller_left"],
            output='screen'
        ),
        # RViz
        Node(
            package='rviz2',
            executable='rviz2',
            # arguments=['-d', rviz_config_path],
            output='screen'
        )
    ])
