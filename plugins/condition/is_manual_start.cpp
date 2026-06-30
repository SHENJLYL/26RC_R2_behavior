#include "r2_behavior/plugins/condition/is_manual_start.hpp"

namespace r2_behavior
{

IsManualStartCondition::IsManualStartCondition(
  const std::string & name, const BT::NodeConfiguration & config,
  std::shared_ptr<std::atomic<int>> manual_start)
: BT::ConditionNode(name, config), manual_start_(std::move(manual_start))
{
}

BT::PortsList IsManualStartCondition::providedPorts()
{
  return {
    BT::InputPort<std::string>("key_port", "{@manual_start}", "Compatibility port"),
    BT::InputPort<int>("start_value", 1, "Minimum value to trigger manual start"),
  };
}

BT::NodeStatus IsManualStartCondition::tick()
{
  int start_value = 1;
  getInput("start_value", start_value);
  return manual_start_->load() >= start_value ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

}  // namespace r2_behavior
