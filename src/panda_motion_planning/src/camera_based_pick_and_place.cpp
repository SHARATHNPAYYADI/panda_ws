#include <rclcpp/rclcpp.hpp>
#include "panda_msgs/srv/move_to_goal.hpp"
#include "panda_msgs/srv/move_joints.hpp"
#include "panda_msgs/srv/move_gripper.hpp"
#include "panda_msgs/srv/detect_object.hpp"
#include "linkattacher_msgs/srv/attach_link.hpp"
#include "linkattacher_msgs/srv/detach_link.hpp"

class CameraSequenceExecutor : public rclcpp::Node
{
public:
    CameraSequenceExecutor() : Node("Camera_sequence_executor")
    {
        // Create clients for all services
        move_joints_client_ = this->create_client<panda_msgs::srv::MoveJoints>("/move_joints");
        move_arms_client_ = this->create_client<panda_msgs::srv::MoveToGoal>("/move_to_goal");
        move_gripper_client_ = this->create_client<panda_msgs::srv::MoveGripper>("/move_gripper");
        attach_client_ = this->create_client<linkattacher_msgs::srv::AttachLink>("/ATTACHLINK");
        detach_client_ = this->create_client<linkattacher_msgs::srv::DetachLink>("/DETACHLINK");
        detect_object_client_ = this->create_client<panda_msgs::srv::DetectObject>("/detect_objects");

        // Wait for services
        RCLCPP_INFO(this->get_logger(), "Waiting for services...");
        move_joints_client_->wait_for_service();
        move_arms_client_->wait_for_service();
        move_gripper_client_->wait_for_service();
        attach_client_->wait_for_service();
        detach_client_->wait_for_service();
        RCLCPP_INFO(this->get_logger(), "All services are available.");

        // Execute the sequence
        camera_based_execute_sequence();
    }

private:
    rclcpp::Client<panda_msgs::srv::MoveJoints>::SharedPtr move_joints_client_;
    rclcpp::Client<panda_msgs::srv::MoveToGoal>::SharedPtr move_arms_client_;
    rclcpp::Client<panda_msgs::srv::MoveGripper>::SharedPtr move_gripper_client_;
    rclcpp::Client<linkattacher_msgs::srv::AttachLink>::SharedPtr attach_client_;
    rclcpp::Client<linkattacher_msgs::srv::DetachLink>::SharedPtr detach_client_;
    rclcpp::Client<panda_msgs::srv::DetectObject>::SharedPtr detect_object_client_;

    void camera_based_execute_sequence()
    {
        // ---------------- Step 1 go to the prepare position ----------------
        call_move_joints({-28, -29 , 23, -100, 43, 154, 20});

        // // ---------------- Step 2 do the position detection using camera----------------
        RCLCPP_INFO(this->get_logger(), "Getting the pose of red object");
        auto position = call_detect_object();
        RCLCPP_INFO(this->get_logger(), "Object Position: x=%.2f, y=%.2f, z=%.2f",
            std::get<0>(position), std::get<1>(position), std::get<2>(position));
        // // ---------------- Step 3 open the gripper----------------
        RCLCPP_INFO(this->get_logger(), "Opening the Gripper");
        call_move_gripper(0.04);

        // // ---------------- Step 4 go to the grasp position----------------
        RCLCPP_INFO(this->get_logger(), "Going to the grasping position");
        call_move_to_goal(std::get<0>(position), std::get<1>(position), std::get<2>(position), 0,0,0,0);

        // // ---------------- Step 5 grasp the object ----------------
        RCLCPP_INFO(this->get_logger(), "Grasping the object");
        call_move_gripper(0.024);
        // // ---------------- Step 6 place it somewhere----------------
        RCLCPP_INFO(this->get_logger(), "attaching the object");

        // // ---------------- Step 7 attach the object ----------------
        RCLCPP_INFO(this->get_logger(), "Moving to placing position");
        call_move_to_goal(0.64, -0.2, 0.2, 0,0,0,0);

        

        // // ---------------- Step 8 Detach and open the gripper----------------
        RCLCPP_INFO(this->get_logger(), "Open the gripper");
        call_move_gripper(0.04);

        RCLCPP_INFO(this->get_logger(), "Camera based pick and place completed");
    }

    // Helper functions
    void call_move_to_goal(double x, double y, double z,
                       double orie_x, double orie_y, double orie_z, double orie_w)
    {
        auto request = std::make_shared<panda_msgs::srv::MoveToGoal::Request>();
        request->x = x;
        request->y = y + 0.015;
        request->z = z*2 + 0.04;
        request->orie_x = orie_x;
        request->orie_y = orie_y;
        request->orie_z = orie_z;
        request->orie_w = orie_w;

        auto result_future = move_arms_client_->async_send_request(request);
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future)
            == rclcpp::FutureReturnCode::SUCCESS)
        {
            auto result = result_future.get();
            if (result->success)
                RCLCPP_INFO(this->get_logger(), "MoveToGoal executed successfully.");
            else
                RCLCPP_ERROR(this->get_logger(), "MoveToGoal failed: %s", result->message.c_str());
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "MoveToGoal service call failed.");
        }
    }


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

    std::tuple<double, double, double> call_detect_object()
    {
        auto request = std::make_shared<panda_msgs::srv::DetectObject::Request>();
        auto future = detect_object_client_->async_send_request(request);

        // Wait for the result (blocking)
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future) ==
            rclcpp::FutureReturnCode::SUCCESS)
        {
            auto response = future.get();
            if (response->success) {
                return std::make_tuple(response->x, response->y, response->z);
            } else {
                RCLCPP_WARN(this->get_logger(), "Detection failed: %s", response->message.c_str());
            }
        } else {
            RCLCPP_ERROR(this->get_logger(), "Service call failed");
        }

        // Return default if failed
        return std::make_tuple(0.0, 0.0, 0.0);
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
    auto node = std::make_shared<CameraSequenceExecutor>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
