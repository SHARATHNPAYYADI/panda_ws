import sys

# ROS2 Python API libraries
import rclpy

# Panda example imports
from .examples.panda_teleop_control import PandaTeleopControl
from .examples.panda_follow_trajectory import PandaFollowTrajectory
from .examples.panda_pick_n_place import PandaPickAndPlace
from .examples.panda_pick_n_insert import PandaPickAndInsert

def main(args=None):
    rclpy.init(args=args)

    if not sys.argv[2]:
        raise ValueError("[runner.py] Error: Unrecognized arguments passed to node in call to `ros2 launch panda_bringup bringup.launch.py mode:=<option>`")

    if "follow" in sys.argv[2]:
        node = PandaFollowTrajectory()
    elif "picknplace" in sys.argv[2]:
        node = PandaPickAndPlace()
    elif "pickninsert" in sys.argv[2]:
        node = PandaPickAndInsert()
    elif "teleop" in sys.argv[2]:
        node = PandaTeleopControl()
    else:
        raise ValueError("[runner.py] Error: Unrecognized arguments passed to node in call to `ros2 launch panda_bringup bringup.launch.py mode:=<option>`. Valid options for <option> are `follow`, `picknplace`, `pickninsert`, or `teleop`")

    rclpy.spin(node)

    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically
    # when the garbage collector destroys the node object)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()