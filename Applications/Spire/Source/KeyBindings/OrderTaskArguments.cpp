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

optional<const OrderTaskArguments&> Spire::find_order_task_arguments(
    const OrderTaskArgumentsListModel& arguments, const Region& region,
    const QKeySequence& key) {
  return none;
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
