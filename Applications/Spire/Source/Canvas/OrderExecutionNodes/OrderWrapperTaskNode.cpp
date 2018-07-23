#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Nexus/Tasks/SingleOrderTask.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/Types/TaskType.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tasks;
using namespace Spire;
using namespace std;

OrderWrapperTaskNode::OrderWrapperTaskNode(const Order& order,
    const UserProfile& userProfile)
    : m_order(&order) {
  Initialize("Single Order", userProfile);
}

OrderWrapperTaskNode::OrderWrapperTaskNode(const Order& order,
    const UserProfile& userProfile, string text)
    : m_order(&order) {
  Initialize(std::move(text), userProfile);
}

const Order& OrderWrapperTaskNode::GetOrder() const {
  return *m_order;
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
  SetType(TaskType::GetInstance());
  AddChild(BaseSingleOrderTaskFactory::SECURITY,
    make_unique<SecurityNode>(m_order->GetInfo().m_fields.m_security,
    userProfile.GetMarketDatabase()));
  AddChild(BaseSingleOrderTaskFactory::ORDER_TYPE,
    make_unique<OrderTypeNode>(m_order->GetInfo().m_fields.m_type));
  AddChild(BaseSingleOrderTaskFactory::SIDE,
    make_unique<SideNode>(m_order->GetInfo().m_fields.m_side));
  unique_ptr<DestinationNode> destinationNode = LinkedNode::SetReferent(
    DestinationNode(m_order->GetInfo().m_fields.m_destination), "security");
  AddChild(BaseSingleOrderTaskFactory::DESTINATION, std::move(destinationNode));
  auto priceNode = LinkedNode::SetReferent(
    MoneyNode(m_order->GetInfo().m_fields.m_price), "security");
  AddChild(BaseSingleOrderTaskFactory::PRICE, std::move(priceNode));
  auto quantityNode = LinkedNode::SetReferent(
    IntegerNode(m_order->GetInfo().m_fields.m_quantity), "security");
  AddChild(BaseSingleOrderTaskFactory::QUANTITY, std::move(quantityNode));
  AddChild(BaseSingleOrderTaskFactory::CURRENCY,
    make_unique<CurrencyNode>(m_order->GetInfo().m_fields.m_currency,
    userProfile.GetCurrencyDatabase().FromId(
    m_order->GetInfo().m_fields.m_currency).m_code.GetData()));
  AddChild(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
    make_unique<TimeInForceNode>(m_order->GetInfo().m_fields.m_timeInForce));
}
