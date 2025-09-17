#include <rclcpp/rclcpp.hpp>
#include "panda_msgs/srv/move_joints.hpp"
#include "panda_msgs/srv/move_gripper.hpp"
#include "linkattacher_msgs/srv/attach_link.hpp"
#include "linkattacher_msgs/srv/detach_link.hpp"

class SequenceExecutor : public rclcpp::Node
{
public:
    SequenceExecutor() : Node("sequence_executor")
    {
        // Create clients for all services
        move_joints_client_ = this->create_client<panda_msgs::srv::MoveJoints>("/move_joints");
        move_gripper_client_ = this->create_client<panda_msgs::srv::MoveGripper>("/move_gripper");
        attach_client_ = this->create_client<linkattacher_msgs::srv::AttachLink>("/ATTACHLINK");
        detach_client_ = this->create_client<linkattacher_msgs::srv::DetachLink>("/DETACHLINK");

        // Wait for services
        RCLCPP_INFO(this->get_logger(), "Waiting for services...");
        move_joints_client_->wait_for_service();
        move_gripper_client_->wait_for_service();
        attach_client_->wait_for_service();
        detach_client_->wait_for_service();
        RCLCPP_INFO(this->get_logger(), "All services are available.");

        // Execute the sequence
        execute_sequence();
    }

private:
    rclcpp::Client<panda_msgs::srv::MoveJoints>::SharedPtr move_joints_client_;
    rclcpp::Client<panda_msgs::srv::MoveGripper>::SharedPtr move_gripper_client_;
    rclcpp::Client<linkattacher_msgs::srv::AttachLink>::SharedPtr attach_client_;
    rclcpp::Client<linkattacher_msgs::srv::DetachLink>::SharedPtr detach_client_;

    void execute_sequence()
    {
        // ---------------- Step 1 Go to pre grasp position ----------------
        call_move_joints({90, -40 , -114, -130, 147, 132, -146});

        // ---------------- Step 2 Open the gripper----------------
        call_move_gripper(0.04);

        // ---------------- Step 3 Move the joint to grasp the object----------------
        call_move_joints({131, -64 , -128, -70, 145, 155, -144});

        // ---------------- Step 4 Close the gripper----------------
        call_move_gripper(0.024);

        // ---------------- Step 5 Attach the link and end effectore----------------
        call_attach_link("my_robot", "panda_link7", "unit_cylinder", "link");

        // ---------------- Step 6 Move to the second position----------------
        call_move_joints({90, -40 , -114, -130, 147, 132, -146});

        // ---------------- Step 7 Detach the link and open the gripper----------------
        call_detach_link("my_robot", "panda_link7", "unit_cylinder", "link");
        call_move_gripper(0.04);
        RCLCPP_INFO(this->get_logger(), "Sequence execution completed.");
    }

    // Helper functions
    void call_move_joints(const std::vector<double> &angles)
    {
        auto request = std::make_shared<panda_msgs::srv::MoveJoints::Request>();
        request->joint_angles_deg = angles;

        auto result_future = move_joints_client_->async_send_request(request);
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future)
            == rclcpp::FutureReturnCode::SUCCESS)
        {
            auto result = result_future.get();
            if (result->success)
                RCLCPP_INFO(this->get_logger(), "MoveJoints executed successfully.");
            else
                RCLCPP_ERROR(this->get_logger(), "MoveJoints failed: %s", result->message.c_str());
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "MoveJoints service call failed.");
        }
    }

    void call_move_gripper(double position)
    {
        auto request = std::make_shared<panda_msgs::srv::MoveGripper::Request>();
        request->position = position;

        auto result_future = move_gripper_client_->async_send_request(request);
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future)
            == rclcpp::FutureReturnCode::SUCCESS)
        {
            RCLCPP_INFO(this->get_logger(), "MoveGripper executed successfully.");
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "MoveGripper service call failed.");
        }
    }

    void call_attach_link(const std::string &model1, const std::string &link1,
                          const std::string &model2, const std::string &link2)
    {
        auto request = std::make_shared<linkattacher_msgs::srv::AttachLink::Request>();
        request->model1_name = model1;
        request->link1_name = link1;
        request->model2_name = model2;
        request->link2_name = link2;

        auto result_future = attach_client_->async_send_request(request);
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future)
            == rclcpp::FutureReturnCode::SUCCESS)
        {
            RCLCPP_INFO(this->get_logger(), "AttachLink executed successfully.");
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "AttachLink service call failed.");
        }
    }

    void call_detach_link(const std::string &model1, const std::string &link1,
                          const std::string &model2, const std::string &link2)
    {
        auto request = std::make_shared<linkattacher_msgs::srv::DetachLink::Request>();
        request->model1_name = model1;
        request->link1_name = link1;
        request->model2_name = model2;
        request->link2_name = link2;

        auto result_future = detach_client_->async_send_request(request);
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future)
            == rclcpp::FutureReturnCode::SUCCESS)
        {
            RCLCPP_INFO(this->get_logger(), "DetachLink executed successfully.");
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "DetachLink service call failed.");
        }
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SequenceExecutor>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
