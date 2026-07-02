// Copyright 2026 AIM Robotics
//
// Licensed under the Apache License, Version 2.0.

#include "r2_behavior/r2_behavior_server.hpp"

#include <chrono>
#include <filesystem>
#include <memory>

#include "ament_index_cpp/get_package_share_directory.hpp"
#include "r2_behavior/plugins/action/navigate_to_named_pose.hpp"
#include "r2_behavior/plugins/action/pub_nav2_goal.hpp"
#include "r2_behavior/plugins/action/pub_twist.hpp"
#include "r2_behavior/plugins/condition/is_manual_start.hpp"

using namespace std::chrono_literals;

namespace r2_behavior
{

R2BehaviorServer::R2BehaviorServer(const rclcpp::NodeOptions & options)
: Node("r2_behavior_server", options), manual_start_(std::make_shared<std::atomic<int>>(0))
{
  const auto share_dir = ament_index_cpp::get_package_share_directory("r2_behavior");
  declare_parameter<std::string>("behavior_tree_file", "");
  declare_parameter<std::string>("behavior_tree_directory", share_dir + "/behavior_trees");
  declare_parameter<std::string>("target_tree", "R2_Competition_Main");
  declare_parameter<double>("tick_frequency", 5.0);

  get_parameter("behavior_tree_file", behavior_tree_file_);
  get_parameter("behavior_tree_directory", behavior_tree_directory_);
  get_parameter("target_tree", target_tree_);
  const double tick_frequency = get_parameter("tick_frequency").as_double();

  manual_start_sub_ = create_subscription<std_msgs::msg::Int32>(
    "manual_start", rclcpp::QoS(10),
    [this](const std_msgs::msg::Int32::SharedPtr msg) { manual_start_->store(msg->data); });

  registerNodes();
  loadTree();

  const auto period = std::chrono::duration<double>(1.0 / std::max(tick_frequency, 0.1));
  tick_timer_ = create_wall_timer(
    std::chrono::duration_cast<std::chrono::milliseconds>(period),
    std::bind(&R2BehaviorServer::tickTree, this));
}

void R2BehaviorServer::registerNodes()
{
  auto node = std::shared_ptr<rclcpp::Node>(this, [](rclcpp::Node *) {});

  factory_.registerBuilder<IsManualStartCondition>(
    "IsManualStart",
    [this](const std::string & name, const BT::NodeConfiguration & config) {
      return std::make_unique<IsManualStartCondition>(name, config, manual_start_);
    });

  factory_.registerBuilder<PubNav2GoalAction>(
    "PubNav2Goal",
    [node](const std::string & name, const BT::NodeConfiguration & config) {
      return std::make_unique<PubNav2GoalAction>(name, config, node);
    });

  factory_.registerBuilder<PublishTwistAction>(
    "PublishTwist",
    [node](const std::string & name, const BT::NodeConfiguration & config) {
      return std::make_unique<PublishTwistAction>(name, config, node);
    });

  factory_.registerBuilder<NavigateToNamedPoseAction>(
    "NavigateToNamedPose",
    [node](const std::string & name, const BT::NodeConfiguration & config) {
      return std::make_unique<NavigateToNamedPoseAction>(name, config, node);
    });
}

void R2BehaviorServer::loadTree()
{
  if (!behavior_tree_file_.empty()) {
    factory_.registerBehaviorTreeFromFile(behavior_tree_file_);
  } else {
    for (const auto & entry : std::filesystem::directory_iterator(behavior_tree_directory_)) {
      if (entry.is_regular_file() && entry.path().extension() == ".xml") {
        factory_.registerBehaviorTreeFromFile(entry.path().string());
      }
    }
  }

  tree_ = std::make_unique<BT::Tree>(factory_.createTree(target_tree_));
  RCLCPP_INFO(
    get_logger(), "Loaded R2 behavior tree '%s' from '%s'", target_tree_.c_str(),
    behavior_tree_file_.empty() ? behavior_tree_directory_.c_str() : behavior_tree_file_.c_str());
}

void R2BehaviorServer::tickTree()
{
  if (!tree_) {
    return;
  }

  const auto status = tree_->tickRoot();
  if (status != last_status_) {
    RCLCPP_INFO(get_logger(), "R2 behavior tree status changed to %d", static_cast<int>(status));
    last_status_ = status;
  }
}

}  // namespace r2_behavior

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<r2_behavior::R2BehaviorServer>(rclcpp::NodeOptions()));
  rclcpp::shutdown();
  return 0;
}
