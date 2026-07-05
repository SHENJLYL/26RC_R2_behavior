// Copyright 2026 AIM Robotics
//
// Licensed under the Apache License, Version 2.0.

#ifndef R2_BEHAVIOR__R2_BEHAVIOR_CLIENT_HPP_
#define R2_BEHAVIOR__R2_BEHAVIOR_CLIENT_HPP_

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

namespace r2_behavior
{

class R2BehaviorClient : public rclcpp::Node
{
public:
  explicit R2BehaviorClient(const rclcpp::NodeOptions & options);

private:
  void publishStart();

  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr manual_start_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  bool auto_start_{false};
  int start_value_{1};
  int auto_start_repeat_count_{1};
  int auto_start_publish_count_{0};
};

}  // namespace r2_behavior

#endif  // R2_BEHAVIOR__R2_BEHAVIOR_CLIENT_HPP_
