#include "r2_behavior/plugins/action/pub_nav2_goal.hpp"

#include "r2_behavior/custom_types.hpp"

namespace r2_behavior
{

PubNav2GoalAction::PubNav2GoalAction(
  const std::string & name, const BT::NodeConfiguration & config,
  const rclcpp::Node::SharedPtr & node)
: BT::SyncActionNode(name, config), node_(node)
{
}

BT::PortsList PubNav2GoalAction::providedPorts()
{
  return {
    BT::InputPort<std::string>("topic_name", "goal_pose", "PoseStamped topic name"),
    BT::InputPort<geometry_msgs::msg::PoseStamped>(
      "goal", "Goal pose in x;y;yaw or x;y;z;qx;qy;qz;qw format"),
  };
}

BT::NodeStatus PubNav2GoalAction::tick()
{
  std::string topic_name = "goal_pose";
  getInput("topic_name", topic_name);

  const auto goal = getInput<geometry_msgs::msg::PoseStamped>("goal");
  if (!goal) {
    RCLCPP_ERROR(node_->get_logger(), "PubNav2Goal missing required input [goal]");
    return BT::NodeStatus::FAILURE;
  }

  auto publisher_it = publishers_.find(topic_name);
  if (publisher_it == publishers_.end()) {
    publisher_it =
      publishers_.emplace(topic_name, node_->create_publisher<geometry_msgs::msg::PoseStamped>(
                                        topic_name, rclcpp::QoS(10)))
        .first;
  }

  auto msg = goal.value();
  msg.header.stamp = node_->now();
  msg.header.frame_id = "map";
  publisher_it->second->publish(msg);
  return BT::NodeStatus::SUCCESS;
}

}  // namespace r2_behavior
