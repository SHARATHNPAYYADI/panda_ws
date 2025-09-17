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
    cube_urdf = os.path.join(pkg_share, 'description', 'models', 'panda', 'cube.urdf')

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
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(get_package_share_directory('panda_moveit_config'), 'launch', 'demo.launch.py')
            ),
            launch_arguments={'world': simple_world}.items()
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
                controllers_yaml,
                {'use_sim_time': True}  
            ],
            output='screen'
        ),
        # Spawn robot in Gazebo
        Node(
            package='gazebo_ros',
            executable='spawn_entity.py',
            arguments=['-topic', 'robot_description', '-entity', 'my_robot', '-reference_frame', 'world'],
            output='screen',
            parameters=[{
                'use_sim_time': True
            }]
        ),
        Node(
            package='panda_bringup',
            executable='reset_panda_joints',
            output='screen'
        ),
        
    ])
