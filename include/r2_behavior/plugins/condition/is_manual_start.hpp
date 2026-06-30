// Copyright 2025 Lihan Chen
// Copyright 2026 AIM Robotics
//
// Licensed under the Apache License, Version 2.0.

#ifndef R2_BEHAVIOR__PLUGINS__CONDITION__IS_MANUAL_START_HPP_
#define R2_BEHAVIOR__PLUGINS__CONDITION__IS_MANUAL_START_HPP_

#include <atomic>
#include <memory>
#include <string>

#include "behaviortree_cpp_v3/condition_node.h"

namespace r2_behavior
{

class IsManualStartCondition : public BT::ConditionNode
{
public:
  IsManualStartCondition(
    const std::string & name, const BT::NodeConfiguration & config,
    std::shared_ptr<std::atomic<int>> manual_start);

  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  std::shared_ptr<std::atomic<int>> manual_start_;
};

}  // namespace r2_behavior

#endif  // R2_BEHAVIOR__PLUGINS__CONDITION__IS_MANUAL_START_HPP_
