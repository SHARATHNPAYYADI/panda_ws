#include <gtest/gtest.h>
// #include "panda_motion_planning/panda_joint_goal.hpp"
#define TESTING_EXCLUDE_MAIN
#include "../src/panda_joint_goal.cpp"
class MockMoveGroupInterface {
public:
    bool setJointValueTarget(const std::vector<double>& joints) {
        last_target_ = joints;
        return true;
    }

    moveit::core::MoveItErrorCode plan(moveit::planning_interface::MoveGroupInterface::Plan& plan) {
        return moveit::core::MoveItErrorCode::SUCCESS;
    }

    std::vector<double> last_target_;
};

TEST(JointServiceTest, ValidInput) {
    auto node = PandaJointService::create();  // Use factory
    std::vector<double> input = {0, 10, 20, 30, 40, 50, 60};
    auto result = node->validateAndPlan(input);
    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second, "Joint movement planned!");
}

TEST(JointServiceTest, InvalidInputSize) {
    auto node = PandaJointService::create();  // Use factory
    std::vector<double> input = {0, 10, 20};  // too few
    auto result = node->validateAndPlan(input);
    EXPECT_FALSE(result.first);
    EXPECT_EQ(result.second, "You must provide exactly 7 joint angles!");
}
int main(int argc, char **argv)
{
  // Initialize ROS 2 before running tests
  rclcpp::init(argc, argv);

  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();

  // Shutdown ROS 2 cleanly
  rclcpp::shutdown();
  return result;
}