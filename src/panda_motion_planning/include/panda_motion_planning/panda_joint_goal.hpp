#pragma once

#include "rclcpp/rclcpp.hpp"
#include <moveit/move_group_interface/move_group_interface.h>
#include <angles/angles.h>
#include "panda_msgs/srv/move_joints.hpp" 

class PandaJointService : public rclcpp::Node
{
public:
  static std::shared_ptr<PandaJointService> create();

  moveit::planning_interface::MoveGroupInterface::Plan plan;
  PandaJointService();
  std::pair<bool, std::string> validateAndPlan(const std::vector<double>& joint_angles_deg);

private:

  

  void init_move_group();

  void moveJointsCallback(
    const std::shared_ptr<panda_msgs::srv::MoveJoints::Request> request,
    std::shared_ptr<panda_msgs::srv::MoveJoints::Response> response);


  

  std::shared_ptr<moveit::planning_interface::MoveGroupInterface> move_group_;
  rclcpp::Service<panda_msgs::srv::MoveJoints>::SharedPtr service_;
};
