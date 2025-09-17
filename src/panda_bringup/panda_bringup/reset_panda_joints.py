#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from trajectory_msgs.msg import JointTrajectory, JointTrajectoryPoint

class ResetPandaJoints(Node):
    def __init__(self):
        super().__init__('reset_panda_joints')
        self.pub = self.create_publisher(JointTrajectory, '/panda_arm_controller/joint_trajectory', 10)

        msg = JointTrajectory()
        msg.joint_names = [
            'panda_joint1','panda_joint2','panda_joint3',
            'panda_joint4','panda_joint5','panda_joint6','panda_joint7'
        ]
        point = JointTrajectoryPoint()
        point.positions = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        point.time_from_start.sec = 2
        msg.points.append(point)

        # Publish once after a short delay
        self.create_timer(0.5, lambda: self.publish_once(msg))

    def publish_once(self, msg):
        self.pub.publish(msg)
        self.get_logger().info("Published reset joint trajectory")
        rclpy.shutdown()  # Exit node after publishing

def main(args=None):
    rclpy.init(args=args)
    node = ResetPandaJoints()
    rclpy.spin(node)

if __name__ == '__main__':
    main()
