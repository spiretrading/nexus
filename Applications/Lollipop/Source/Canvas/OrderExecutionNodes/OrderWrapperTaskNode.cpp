#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

OrderWrapperTaskNode::OrderWrapperTaskNode(const std::shared_ptr<Order>& order,
    const UserProfile& userProfile)
    : m_order(order) {
  Initialize("Single Order", userProfile);
}

OrderWrapperTaskNode::OrderWrapperTaskNode(const std::shared_ptr<Order>& order,
    const UserProfile& userProfile, string text)
    : m_order(order) {
  Initialize(std::move(text), userProfile);
}

const std::shared_ptr<Order>& OrderWrapperTaskNode::GetOrder() const {
  return m_order;
}

unique_ptr<OrderWrapperTaskNode> OrderWrapperTaskNode::Rename(
    string name) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetText(std::move(name));
  return clone;
}

void OrderWrapperTaskNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> OrderWrapperTaskNode::Clone() const {
  return make_unique<OrderWrapperTaskNode>(*this);
}

void OrderWrapperTaskNode::Initialize(string text,
    const UserProfile& userProfile) {
  SetText(std::move(text));
  SetType(OrderReferenceType::GetInstance());
  AddChild(SingleOrderTaskNode::SECURITY_PROPERTY,
    make_unique<SecurityNode>(m_order->get_info().m_fields.m_security,
    userProfile.GetVenueDatabase()));
  AddChild(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
    make_unique<OrderTypeNode>(m_order->get_info().m_fields.m_type));
  AddChild(SingleOrderTaskNode::SIDE_PROPERTY,
    make_unique<SideNode>(m_order->get_info().m_fields.m_side));
  unique_ptr<DestinationNode> destinationNode = LinkedNode::SetReferent(
    DestinationNode(m_order->get_info().m_fields.m_destination), "security");
  AddChild(SingleOrderTaskNode::DESTINATION_PROPERTY,
    std::move(destinationNode));
  auto priceNode = LinkedNode::SetReferent(
    MoneyNode(m_order->get_info().m_fields.m_price), "security");
  AddChild(SingleOrderTaskNode::PRICE_PROPERTY, std::move(priceNode));
  auto quantityNode = LinkedNode::SetReferent(
    IntegerNode(m_order->get_info().m_fields.m_quantity), "security");
  AddChild(SingleOrderTaskNode::QUANTITY_PROPERTY, std::move(quantityNode));
  AddChild(SingleOrderTaskNode::CURRENCY_PROPERTY,
    make_unique<CurrencyNode>(m_order->get_info().m_fields.m_currency,
    userProfile.GetCurrencyDatabase().from(
    m_order->get_info().m_fields.m_currency).m_code.get_data()));
  AddChild(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
    make_unique<TimeInForceNode>(m_order->get_info().m_fields.m_time_in_force));
}
