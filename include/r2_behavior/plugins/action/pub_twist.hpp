// Copyright 2025 Lihan Chen
// Copyright 2026 AIM Robotics
//
// Licensed under the Apache License, Version 2.0.

#ifndef R2_BEHAVIOR__PLUGINS__ACTION__PUB_TWIST_HPP_
#define R2_BEHAVIOR__PLUGINS__ACTION__PUB_TWIST_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include "behaviortree_cpp_v3/action_node.h"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"

namespace r2_behavior
{

class PublishTwistAction : public BT::SyncActionNode
{
public:
  PublishTwistAction(
    const std::string & name, const BT::NodeConfiguration & config,
    const rclcpp::Node::SharedPtr & node);

  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  rclcpp::Node::SharedPtr node_;
  std::unordered_map<std::string, rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr>
    publishers_;
};

}  // namespace r2_behavior

#endif  // R2_BEHAVIOR__PLUGINS__ACTION__PUB_TWIST_HPP_
