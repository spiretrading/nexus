#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/TaskVolumeNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Nexus;
using namespace Spire;
using namespace std;

const std::string SingleOrderTaskNode::SECURITY_PROPERTY = "security";

const std::string SingleOrderTaskNode::ORDER_TYPE_PROPERTY = "type";

const std::string SingleOrderTaskNode::SIDE_PROPERTY = "side";

const std::string SingleOrderTaskNode::DESTINATION_PROPERTY = "destination";

const std::string SingleOrderTaskNode::PRICE_PROPERTY = "price";

const std::string SingleOrderTaskNode::QUANTITY_PROPERTY = "quantity";

const std::string SingleOrderTaskNode::CURRENCY_PROPERTY = "currency";

const std::string SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY = "time_in_force";

SingleOrderTaskNode::FieldEntry::FieldEntry()
  : m_key(-1) {}

SingleOrderTaskNode::FieldEntry::FieldEntry(string name, const NativeType& type,
  int key)
  : m_name(std::move(name)),
    m_type(type),
    m_key(key) {}

SingleOrderTaskNode::SingleOrderTaskNode()
    : SingleOrderTaskNode("Single Order") {}

SingleOrderTaskNode::SingleOrderTaskNode(string text) {
  SetText(std::move(text));
  SetType(OrderReferenceType::GetInstance());
  AddChild(SECURITY_PROPERTY, make_unique<SecurityNode>());
  AddChild(ORDER_TYPE_PROPERTY, make_unique<OrderTypeNode>(OrderType::LIMIT));
  AddChild(SIDE_PROPERTY, make_unique<SideNode>());
  unique_ptr<DestinationNode> destinationNode = LinkedNode::SetReferent(
    DestinationNode(), "security");
  AddChild(DESTINATION_PROPERTY, std::move(destinationNode));
  auto priceNode = LinkedNode::SetReferent(MoneyNode(), "security");
  AddChild(PRICE_PROPERTY, std::move(priceNode));
  auto quantityNode = LinkedNode::SetReferent(IntegerNode(), "security");
  AddChild(QUANTITY_PROPERTY, std::move(quantityNode));
  unique_ptr<CanvasNode> currencyNode = make_unique<DefaultCurrencyNode>();
  currencyNode = currencyNode->Replace(currencyNode->GetChildren().front(),
    make_unique<ReferenceNode>("<security"))->SetVisible(false);
  AddChild(CURRENCY_PROPERTY, std::move(currencyNode));
  AddChild(TIME_IN_FORCE_PROPERTY, make_unique<TimeInForceNode>());
}

const vector<SingleOrderTaskNode::FieldEntry>&
    SingleOrderTaskNode::GetFields() const {
  return m_fields;
}

unique_ptr<SingleOrderTaskNode> SingleOrderTaskNode::AddField(
    string name, int key, unique_ptr<CanvasNode> value) const {
  if(dynamic_cast<const NativeType*>(&value->GetType()) == nullptr) {
    BOOST_THROW_EXCEPTION(CanvasOperationException(
      "Fields must be of a native type."));
  }
  auto clone = CanvasNode::Clone(*this);
  FieldEntry entry(name, dynamic_cast<const NativeType&>(value->GetType()),
    key);
  clone->m_fields.push_back(entry);
  clone->AddChild(std::move(name), std::move(value));
  return clone;
}

unique_ptr<SingleOrderTaskNode> SingleOrderTaskNode::Rename(string name) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetText(std::move(name));
  return clone;
}

unique_ptr<CanvasNode> SingleOrderTaskNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  if(&child == &*FindChild(DESTINATION_PROPERTY) &&
      dynamic_cast<const DestinationNode*>(replacement.get()) != nullptr) {
    auto destinationReplacement =
      static_cast<const DestinationNode*>(replacement.get());
    auto destination = LinkedNode::SetReferent(*destinationReplacement,
      "security");
    return CanvasNode::Replace(child, std::move(destination));
  }
  return CanvasNode::Replace(child, std::move(replacement));
}

void SingleOrderTaskNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> SingleOrderTaskNode::Clone() const {
  return make_unique<SingleOrderTaskNode>(*this);
}

SingleOrderTaskNode::SingleOrderTaskNode(ReceiveBuilder) {}
