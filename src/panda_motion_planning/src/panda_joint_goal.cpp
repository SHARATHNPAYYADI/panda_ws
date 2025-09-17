#include "panda_motion_planning/panda_joint_goal.hpp"

// Factory function
std::shared_ptr<PandaJointService> PandaJointService::create()
{
    auto node = std::shared_ptr<PandaJointService>(new PandaJointService());
    node->init_move_group();
    return node;
}

// Constructor
PandaJointService::PandaJointService() : Node("panda_joint_service") {}

/**
 * @brief Validates the plan based on joint angles given
 *
 * This function plans the path and executes it when the plan is valid
 *
 * @param joint_angles_deg A vector of joint angles in angles.
 * @return std::pair<bool, std::string>
 *         - first: true if the plan is valid and successfully generated, false otherwise
 *         - second: message describing the result
 */
std::pair<bool, std::string> PandaJointService::validateAndPlan(const std::vector<double>& joint_angles_deg)
{
    if (joint_angles_deg.size() != 7)
        return {false, "You must provide exactly 7 joint angles!"};

    std::vector<double> target_rad;
    for (double d : joint_angles_deg)
        target_rad.push_back(angles::from_degrees(d));

    move_group_->setJointValueTarget(target_rad);
    bool success = (move_group_->plan(plan) == moveit::core::MoveItErrorCode::SUCCESS);

    if (success)
        return {true, "Joint movement planned!"};
    else
        return {false, "Planning failed!"};
}

/**
 * @brief Initialization of interface and services
 */
void PandaJointService::init_move_group()
{
    move_group_ = std::make_shared<moveit::planning_interface::MoveGroupInterface>(
        shared_from_this(), "panda_arm");

    service_ = this->create_service<panda_msgs::srv::MoveJoints>(
        "move_joints",
        std::bind(&PandaJointService::moveJointsCallback, this,
                  std::placeholders::_1, std::placeholders::_2)
    );

    RCLCPP_INFO(this->get_logger(), "Panda joint service ready...");
}

/**
 * @brief Callback for Service /move_joints
 *
 * This function taeks the request with joint angles and validates the plan and executes it
 *
 * @param request joint angles.
 *        responce responce for service
 */
void PandaJointService::moveJointsCallback(
    const std::shared_ptr<panda_msgs::srv::MoveJoints::Request> request,
    std::shared_ptr<panda_msgs::srv::MoveJoints::Response> response)
{
    bool success = validateAndPlan(request->joint_angles_deg).first;
    if (success)
    {
        move_group_->execute(plan);
        response->success = true;
        response->message = "Joint movement executed!";
        RCLCPP_INFO(this->get_logger(), "Executed joint movement.");
    }
    else
    {
        response->success = false;
        response->message = "Planning failed!";
        RCLCPP_ERROR(this->get_logger(), "Planning failed.");
    }
}

#ifndef TESTING_EXCLUDE_MAIN
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = PandaJointService::create();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
#endif