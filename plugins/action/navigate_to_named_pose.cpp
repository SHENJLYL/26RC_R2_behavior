// Copyright 2026 AIM Robotics
//
// Licensed under the Apache License, Version 2.0.

#include "r2_behavior/plugins/action/navigate_to_named_pose.hpp"

using namespace std::chrono_literals;

namespace r2_behavior
{

NavigateToNamedPoseAction::NavigateToNamedPoseAction(
  const std::string & name, const BT::NodeConfiguration & config,
  const rclcpp::Node::SharedPtr & node)
: BT::StatefulActionNode(name, config), node_(node)
{
  std::string action_name = "/r2/navigation/navigate_to_named_pose";
  getInput("action_name", action_name);
  client_ = rclcpp_action::create_client<ActionT>(node_, action_name);
}

BT::PortsList NavigateToNamedPoseAction::providedPorts()
{
  return {
    BT::InputPort<std::string>(
      "action_name", "/r2/navigation/navigate_to_named_pose", "NavigateToNamedPose action name"),
    BT::InputPort<std::string>("target", "Named waypoint target"),
    BT::InputPort<std::string>("team_side", "red", "Team side in waypoint file"),
    BT::InputPort<double>("timeout_sec", 0.0, "Navigation timeout in seconds"),
    BT::OutputPort<std::string>("error_code", "Navigation error code"),
    BT::OutputPort<std::string>("message", "Navigation result message"),
  };
}

BT::NodeStatus NavigateToNamedPoseAction::onStart()
{
  ActionT::Goal goal;
  getInput("target", goal.target);
  getInput("team_side", goal.team_side);
  getInput("timeout_sec", timeout_sec_);
  goal.timeout_sec = static_cast<float>(timeout_sec_);

  if (goal.target.empty()) {
    setOutput("error_code", "UNKNOWN_WAYPOINT");
    setOutput("message", "target port is empty");
    return BT::NodeStatus::FAILURE;
  }

  if (!client_->wait_for_action_server(500ms)) {
    setOutput("error_code", "NAV2_UNAVAILABLE");
    setOutput("message", "NavigateToNamedPose action server unavailable");
    return BT::NodeStatus::FAILURE;
  }

  started_at_ = std::chrono::steady_clock::now();
  send_goal_future_ = client_->async_send_goal(goal);
  goal_handle_.reset();
  result_requested_ = false;
  setOutput("error_code", "RUNNING");
  setOutput("message", "Navigation goal sent");
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus NavigateToNamedPoseAction::onRunning()
{
  if (timeout_sec_ > 0.0) {
    const auto elapsed = std::chrono::duration<double>(
      std::chrono::steady_clock::now() - started_at_).count();
    if (elapsed > timeout_sec_) {
      if (goal_handle_) {
        client_->async_cancel_goal(goal_handle_);
      }
      setOutput("error_code", "TIMEOUT");
      setOutput("message", "NavigateToNamedPose timed out in BT node");
      return BT::NodeStatus::FAILURE;
    }
  }

  if (!result_requested_) {
    if (send_goal_future_.wait_for(0ms) != std::future_status::ready) {
      return BT::NodeStatus::RUNNING;
    }

    goal_handle_ = send_goal_future_.get();
    if (!goal_handle_) {
      setOutput("error_code", "NAV2_ABORTED");
      setOutput("message", "NavigateToNamedPose goal rejected");
      return BT::NodeStatus::FAILURE;
    }

    result_future_ = client_->async_get_result(goal_handle_);
    result_requested_ = true;
  }

  if (result_future_.wait_for(0ms) != std::future_status::ready) {
    return BT::NodeStatus::RUNNING;
  }

  const auto wrapped = result_future_.get();
  const auto result = wrapped.result;
  setOutput("error_code", result->error_code);
  setOutput("message", result->message);
  return result->success ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

void NavigateToNamedPoseAction::onHalted()
{
  if (goal_handle_) {
    client_->async_cancel_goal(goal_handle_);
    goal_handle_.reset();
  }
}

}  // namespace r2_behavior
