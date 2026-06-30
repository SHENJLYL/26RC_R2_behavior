// Copyright 2026 AIM Robotics
//
// Licensed under the Apache License, Version 2.0.

#ifndef R2_BEHAVIOR__R2_BEHAVIOR_SERVER_HPP_
#define R2_BEHAVIOR__R2_BEHAVIOR_SERVER_HPP_

#include <atomic>
#include <memory>
#include <string>

#include "behaviortree_cpp_v3/bt_factory.h"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

namespace r2_behavior
{

class R2BehaviorServer : public rclcpp::Node
{
public:
  explicit R2BehaviorServer(const rclcpp::NodeOptions & options);

private:
  void registerNodes();
  void loadTree();
  void tickTree();

  std::shared_ptr<std::atomic<int>> manual_start_;
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr manual_start_sub_;
  rclcpp::TimerBase::SharedPtr tick_timer_;
  BT::BehaviorTreeFactory factory_;
  std::unique_ptr<BT::Tree> tree_;
  BT::NodeStatus last_status_{BT::NodeStatus::IDLE};
  std::string behavior_tree_file_;
  std::string behavior_tree_directory_;
  std::string target_tree_;
};

}  // namespace r2_behavior

#endif  // R2_BEHAVIOR__R2_BEHAVIOR_SERVER_HPP_
