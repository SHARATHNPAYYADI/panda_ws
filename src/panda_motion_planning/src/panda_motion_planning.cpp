#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include "panda_msgs/srv/move_to_goal.hpp"
#include "panda_msgs/srv/move_gripper.hpp"
#include <control_msgs/action/gripper_command.hpp>
#include <tf2/LinearMath/Quaternion.h>
using GripperCommand = control_msgs::action::GripperCommand;
using GoalHandleGripper = rclcpp_action::ClientGoalHandle<GripperCommand>;


class PandaMotionPlanner : public rclcpp::Node
{
public:
  PandaMotionPlanner()
  : Node("panda_motion_planner")
  {
    // move_group_ = std::make_shared<moveit::planning_interface::MoveGroupInterface>(shared_from_this(), "panda_arm");
    // Expose service
    move_service_ = this->create_service<panda_msgs::srv::MoveToGoal>(
      "move_to_goal",
      std::bind(&PandaMotionPlanner::moveServiceCallback, this, std::placeholders::_1, std::placeholders::_2)
    );
    gripper_service_ = this->create_service<panda_msgs::srv::MoveGripper>(
      "move_gripper",
      std::bind(&PandaMotionPlanner::gripperServiceCallback, this, std::placeholders::_1, std::placeholders::_2)
    );

    gripper_left_client_ = rclcpp_action::create_client<GripperCommand>(
      this, "/panda_gripper_controller_left/gripper_cmd"
    );
    gripper_right_client_ = rclcpp_action::create_client<GripperCommand>(
      this, "/panda_gripper_controller_right/gripper_cmd"
    );
  }
  // Initialization of Interface and make the robot to start up pose
  void setup()
  {
    move_group_ = std::make_shared<moveit::planning_interface::MoveGroupInterface>(shared_from_this(), "panda_arm");
    start_up_pos();
  }
 /**
 * @brief Makes the robot to go to start up position defined
 */
  void start_up_pos(){
    geometry_msgs::msg::Pose target_pose;
    target_pose.position.x = 0.087;
    target_pose.position.y = -0.0;
    target_pose.position.z = 0.925;

    // Rotate so gripper approaches from the side
    // Here: Z (fingers) -> +X direction
    tf2::Quaternion q;
    target_pose.orientation.x = 0.705; //orie_x; //
    target_pose.orientation.y = -0.031; //orie_y; //
    target_pose.orientation.z = 0.707; //orie_z; //
    target_pose.orientation.w = 0.048; //orie_w; //

    move_group_->setPoseTarget(target_pose);

    moveit::planning_interface::MoveGroupInterface::Plan plan;
    bool success = static_cast<bool>(move_group_->plan(plan));

    if (success) {
      RCLCPP_INFO(this->get_logger(), "Executing plan...");
      move_group_->execute(plan);
    } else {
      RCLCPP_ERROR(this->get_logger(), "Planning failed!");
    }
  }
/**
 * @brief Validates and informs if the robot can be moved to particular position
 *
 * This function plans the path and executes it when the plan is valid
 *
 * @param position x,y,z,orientation_x, orientation_y, orientation_z.
 * @return true if the robot can be moved to particular position
 */
bool moveTo(double x, double y, double z, double orie_x, double orie_y, double orie_z, double orie_w)
{
  geometry_msgs::msg::Pose target_pose;
  target_pose.position.x = x;
  target_pose.position.y = y;
  target_pose.position.z = z;

  //Side-approach orientation (same as startup pose)
  tf2::Quaternion q;
  // q.setRPY(0, M_PI / 2, 0);  // roll=0, pitch=90°, yaw=-45°
  q.setRPY(M_PI, 0, -M_PI/2);
  q.normalize();

  target_pose.orientation.x = q.x();
  target_pose.orientation.y = q.y();
  target_pose.orientation.z = q.z();
  target_pose.orientation.w = q.w();
  

  move_group_->setPoseTarget(target_pose);

  moveit::planning_interface::MoveGroupInterface::Plan plan;
  bool success = static_cast<bool>(move_group_->plan(plan));

  if (success) {
    RCLCPP_INFO(this->get_logger(), "Executing grasp plan...");
    move_group_->execute(plan);
  } else {
    RCLCPP_ERROR(this->get_logger(), "Grasp planning failed!");
  }
  return success;
}




private:
  // Service callback for service /move_to_goal
  void moveServiceCallback(
    const std::shared_ptr<panda_msgs::srv::MoveToGoal::Request> request,
    std::shared_ptr<panda_msgs::srv::MoveToGoal::Response> response)
  {
    auto success = moveTo(request->x, request->y, request->z, request->orie_x, request->orie_y, request->orie_z, request->orie_w);
    response->success = success;
    if(success){
      response->message = "Moved robot to requested position";
    }
    else{
      response->message = "Planning failed";
    }
    
  }
  //service callback for gripper movement
  void gripperServiceCallback(
    const std::shared_ptr<panda_msgs::srv::MoveGripper::Request> request,
    std::shared_ptr<panda_msgs::srv::MoveGripper::Response> response)
  {
    GripperCommand::Goal goal;
    goal.command.max_effort = 0;
    goal.command.position = request->position;

    
    auto send_goal_options = rclcpp_action::Client<GripperCommand>::SendGoalOptions();

    send_goal_options.result_callback = [this, response](const GoalHandleGripper::WrappedResult &result) {
        // Print result code
            switch (result.code) {
                case rclcpp_action::ResultCode::SUCCEEDED:
                    RCLCPP_INFO(this->get_logger(), "Result code: SUCCEEDED");
                    break;
                case rclcpp_action::ResultCode::ABORTED:
                    RCLCPP_INFO(this->get_logger(), "Result code: ABORTED");
                    break;
                case rclcpp_action::ResultCode::CANCELED:
                    RCLCPP_INFO(this->get_logger(), "Result code: CANCELED");
                    break;
                case rclcpp_action::ResultCode::UNKNOWN:
                    RCLCPP_INFO(this->get_logger(), "Result code: UNKNOWN");
                    break;
                default:
                    RCLCPP_INFO(this->get_logger(), "Result code: OTHER");
            }
            
            // Print gripper position if available
            if (result.result) {
                RCLCPP_INFO(this->get_logger(), "Gripper position: %f", result.result->position);
            }
            
        };
    if(request->position <= 0.04 && request->position >= 0.00){
          response->success = true;
          response->message = "Gripper moved successfully";
    }
    else{
      response->success = false;
      response->message = "Gripper failed to move";
    }


    gripper_left_client_->async_send_goal(goal, send_goal_options);
    gripper_right_client_->async_send_goal(goal, send_goal_options);
  }


  std::shared_ptr<moveit::planning_interface::MoveGroupInterface> move_group_;
  rclcpp::Service<panda_msgs::srv::MoveToGoal>::SharedPtr move_service_;
  rclcpp::Service<panda_msgs::srv::MoveGripper>::SharedPtr gripper_service_;
  rclcpp_action::Client<GripperCommand>::SharedPtr gripper_left_client_; 
  rclcpp_action::Client<GripperCommand>::SharedPtr gripper_right_client_;
};
#ifndef TESTING_EXCLUDE_MAIN
int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<PandaMotionPlanner>();
  node->setup();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
#endif
