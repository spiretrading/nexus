#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  template<typename T>
  typename T::Type extract(const optional<const CanvasNode&>& child) {
    if(!child) {
      return {};
    } else if(auto node = dynamic_cast<const ValueNode<T>*>(&*child)) {
      return node->GetValue();
    }
    return {};
  }
}

optional<const OrderTaskArguments&> Spire::find_order_task_arguments(
    const OrderTaskArgumentsListModel& arguments, const Region& region,
    const QKeySequence& key) {
  auto candidates = std::vector<int>();
  for(auto i = 0; i != arguments.get_size(); ++i) {
    auto& argument = arguments.get(i);
    if(argument.m_key == key && region <= argument.m_region) {
      candidates.push_back(i);
    }
  }
  if(candidates.empty()) {
    return none;
  }
  std::sort(candidates.begin(), candidates.end(), [&] (auto left, auto right) {
    return arguments.get(left).m_region < arguments.get(right).m_region;
  });
  return arguments.get(candidates.front());
}

std::unique_ptr<CanvasNode>
    Spire::make_canvas_node(const OrderTaskArguments& arguments) {
  auto node = std::unique_ptr<CanvasNode>(
    std::make_unique<SingleOrderTaskNode>()->Rename(
      arguments.m_name.toStdString()));
  if(arguments.m_order_type != OrderType::NONE) {
    node = node->Replace(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
      std::make_unique<OrderTypeNode>(arguments.m_order_type));
  }
  if(arguments.m_side != Side::NONE) {
    node = node->Replace(SingleOrderTaskNode::SIDE_PROPERTY,
      std::make_unique<SideNode>(arguments.m_side));
  }
  if(!arguments.m_destination.empty()) {
    node = node->Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(arguments.m_destination));
  }
  if(arguments.m_quantity) {
    node = node->Replace(SingleOrderTaskNode::QUANTITY_PROPERTY,
      std::make_unique<IntegerNode>(*arguments.m_quantity));
  }
  if(arguments.m_time_in_force.GetType() != TimeInForce::Type::NONE) {
    node = node->Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(arguments.m_time_in_force));
  }
  return node;
}

OrderTaskArguments Spire::to_order_task_arguments(const CanvasNode& node) {
  auto arguments = OrderTaskArguments();
  arguments.m_name = QString::fromStdString(node.GetText());
  arguments.m_region = Region(DefaultMarkets::ASX(), DefaultCountries::AU());
  arguments.m_region.SetName("ASX");
  arguments.m_destination = extract<DestinationType>(
    node.FindChild(SingleOrderTaskNode::DESTINATION_PROPERTY));
  arguments.m_order_type = extract<OrderTypeType>(
    node.FindChild(SingleOrderTaskNode::ORDER_TYPE_PROPERTY));
  arguments.m_side =
    extract<SideType>(node.FindChild(SingleOrderTaskNode::SIDE_PROPERTY));
  arguments.m_time_in_force = extract<TimeInForceType>(
    node.FindChild(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY));
  return arguments;
}
