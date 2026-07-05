// Copyright 2026 AIM Robotics
//
// Licensed under the Apache License, Version 2.0.

#include "r2_behavior/r2_behavior_client.hpp"

#include <chrono>

using namespace std::chrono_literals;

namespace r2_behavior
{

R2BehaviorClient::R2BehaviorClient(const rclcpp::NodeOptions & options)
: Node("r2_behavior_client", options)
{
  declare_parameter<bool>("auto_start", false);
  declare_parameter<int>("start_value", 1);
  declare_parameter<int>("auto_start_repeat_count", 1);
  get_parameter("auto_start", auto_start_);
  get_parameter("start_value", start_value_);
  get_parameter("auto_start_repeat_count", auto_start_repeat_count_);

  manual_start_pub_ = create_publisher<std_msgs::msg::Int32>("manual_start", rclcpp::QoS(10));

  if (auto_start_) {
    timer_ = create_wall_timer(500ms, std::bind(&R2BehaviorClient::publishStart, this));
  } else {
    RCLCPP_INFO(
      get_logger(),
      "R2 behavior client is idle. Publish /manual_start or set auto_start:=true to start.");
  }
}

void R2BehaviorClient::publishStart()
{
  std_msgs::msg::Int32 msg;
  msg.data = start_value_;
  manual_start_pub_->publish(msg);
  RCLCPP_INFO(get_logger(), "Published manual_start=%d", start_value_);
  ++auto_start_publish_count_;
  if (auto_start_publish_count_ >= std::max(auto_start_repeat_count_, 1)) {
    timer_->cancel();
  }
}

}  // namespace r2_behavior

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<r2_behavior::R2BehaviorClient>(rclcpp::NodeOptions()));
  rclcpp::shutdown();
  return 0;
}
