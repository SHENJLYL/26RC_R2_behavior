// Copyright 2026 AIM Robotics
//
// Licensed under the Apache License, Version 2.0.

#ifndef R2_BEHAVIOR__PLUGINS__ACTION__NAVIGATE_TO_NAMED_POSE_HPP_
#define R2_BEHAVIOR__PLUGINS__ACTION__NAVIGATE_TO_NAMED_POSE_HPP_

#include <chrono>
#include <future>
#include <memory>
#include <string>

#include "behaviortree_cpp_v3/action_node.h"
#include "r2_nav_interfaces/action/navigate_to_named_pose.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

namespace r2_behavior
{

class NavigateToNamedPoseAction : public BT::StatefulActionNode
{
public:
  using ActionT = r2_nav_interfaces::action::NavigateToNamedPose;
  using GoalHandleT = rclcpp_action::ClientGoalHandle<ActionT>;

  NavigateToNamedPoseAction(
    const std::string & name, const BT::NodeConfiguration & config,
    const rclcpp::Node::SharedPtr & node);

  static BT::PortsList providedPorts();
  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp_action::Client<ActionT>::SharedPtr client_;
  std::shared_future<GoalHandleT::SharedPtr> send_goal_future_;
  std::shared_future<GoalHandleT::WrappedResult> result_future_;
  GoalHandleT::SharedPtr goal_handle_;
  std::chrono::steady_clock::time_point started_at_;
  double timeout_sec_{0.0};
  bool result_requested_{false};
};

}  // namespace r2_behavior

#endif  // R2_BEHAVIOR__PLUGINS__ACTION__NAVIGATE_TO_NAMED_POSE_HPP_
