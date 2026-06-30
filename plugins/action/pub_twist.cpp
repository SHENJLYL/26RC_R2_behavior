#include "r2_behavior/plugins/action/pub_twist.hpp"

namespace r2_behavior
{

PublishTwistAction::PublishTwistAction(
  const std::string & name, const BT::NodeConfiguration & config,
  const rclcpp::Node::SharedPtr & node)
: BT::SyncActionNode(name, config), node_(node)
{
}

BT::PortsList PublishTwistAction::providedPorts()
{
  return {
    BT::InputPort<std::string>("topic_name", "cmd_vel", "Twist topic name"),
    BT::InputPort<double>("duration", 0.0, "Compatibility port; currently publishes once per tick"),
    BT::InputPort<double>("v_x", 0.0, "Linear X velocity in m/s"),
    BT::InputPort<double>("v_y", 0.0, "Linear Y velocity in m/s"),
    BT::InputPort<double>("v_yaw", 0.0, "Angular Z velocity in rad/s"),
  };
}

BT::NodeStatus PublishTwistAction::tick()
{
  std::string topic_name = "cmd_vel";
  double vx = 0.0;
  double vy = 0.0;
  double vyaw = 0.0;
  getInput("topic_name", topic_name);
  getInput("v_x", vx);
  getInput("v_y", vy);
  getInput("v_yaw", vyaw);

  auto publisher_it = publishers_.find(topic_name);
  if (publisher_it == publishers_.end()) {
    publisher_it =
      publishers_.emplace(topic_name, node_->create_publisher<geometry_msgs::msg::Twist>(
                                        topic_name, rclcpp::QoS(10)))
        .first;
  }

  geometry_msgs::msg::Twist msg;
  msg.linear.x = vx;
  msg.linear.y = vy;
  msg.angular.z = vyaw;
  publisher_it->second->publish(msg);
  return BT::NodeStatus::SUCCESS;
}

}  // namespace r2_behavior
