#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <moveit/planning_scene_monitor/planning_scene_monitor.h>
#include <moveit/robot_state/robot_state.h>
#include <moveit/robot_model_loader/robot_model_loader.h>
#include <geometry_msgs/msg/pose_stamped.hpp>

class FKPoseLogger : public rclcpp::Node
{
public:
  FKPoseLogger() : Node("fk_pose_logger")
  {
    joint_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
      "/joint_states", 10,
      std::bind(&FKPoseLogger::jointCallback, this, std::placeholders::_1)
    );
  }
  void setup()
  {
    // Use node's shared pointer, not enable_shared_from_this
    robot_model_loader_ = std::make_shared<robot_model_loader::RobotModelLoader>(shared_from_this(), "robot_description");
    robot_model_ = robot_model_loader_->getModel();
    robot_state_ = std::make_shared<moveit::core::RobotState>(robot_model_);
    robot_state_->setToDefaultValues();
  }

private:
  // calculates the position from the joint_state topic
  void jointCallback(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    robot_state_->setVariablePositions(msg->name, msg->position);
    robot_state_->update();

    const auto& transform = robot_state_->getGlobalLinkTransform("panda_hand");

    RCLCPP_INFO(this->get_logger(), "EE Position: x=%.3f y=%.3f z=%.3f",
                transform.translation().x(),
                transform.translation().y(),
                transform.translation().z());

    const auto& quat = Eigen::Quaterniond(transform.rotation());
    RCLCPP_INFO(this->get_logger(), "Orientation: x=%.3f y=%.3f z=%.3f w=%.3f",
                quat.x(), quat.y(), quat.z(), quat.w());
  }

  std::shared_ptr<robot_model_loader::RobotModelLoader> robot_model_loader_;
  moveit::core::RobotModelPtr robot_model_;
  moveit::core::RobotStatePtr robot_state_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_sub_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<FKPoseLogger>();
  node->setup();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
