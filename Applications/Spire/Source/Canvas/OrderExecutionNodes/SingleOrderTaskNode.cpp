#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Nexus/OrderTasks/SingleOrderTask.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/TaskVolumeNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/Types/TaskType.hpp"
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
using namespace Nexus::OrderTasks;
using namespace Spire;
using namespace std;

namespace {
  unique_ptr<CanvasNode> BuildVolumeNode() {
    auto volumeNode = BuildTaskVolumeNode();
    volumeNode = volumeNode->Replace(volumeNode->GetChildren().back(),
      make_unique<ReferenceNode>("<<<", TaskType::GetInstance()));
    unique_ptr<CanvasNode> displayNode = make_unique<AdditionNode>();
    displayNode = displayNode->Replace(displayNode->GetChildren().front(),
      std::move(volumeNode));
    displayNode = displayNode->Replace(displayNode->GetChildren().back(),
      make_unique<ReferenceNode>("<display", IntegerType::GetInstance()));
    return displayNode;
  }
}

const std::string SingleOrderTaskNode::DISPLAY_PROPERTY = "display";
const std::string SingleOrderTaskNode::VOLUME_PROPERTY = "volume";

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
  SetType(TaskType::GetInstance());
  AddChild(BaseSingleOrderTaskFactory::SECURITY, make_unique<SecurityNode>());
  AddChild(BaseSingleOrderTaskFactory::ORDER_TYPE,
    make_unique<OrderTypeNode>(OrderType::LIMIT));
  AddChild(BaseSingleOrderTaskFactory::SIDE, make_unique<SideNode>());
  unique_ptr<DestinationNode> destinationNode = LinkedNode::SetReferent(
    DestinationNode(), "security");
  AddChild(BaseSingleOrderTaskFactory::DESTINATION, std::move(destinationNode));
  auto priceNode = LinkedNode::SetReferent(MoneyNode(), "security");
  AddChild(BaseSingleOrderTaskFactory::PRICE, std::move(priceNode));
  auto quantityNode = LinkedNode::SetReferent(IntegerNode(), "security");
  AddChild(BaseSingleOrderTaskFactory::QUANTITY, std::move(quantityNode));
  AddChild(DISPLAY_PROPERTY,
    make_unique<ReferenceNode>("quantity", IntegerType::GetInstance()));
  unique_ptr<CanvasNode> currencyNode = make_unique<DefaultCurrencyNode>();
  currencyNode = currencyNode->Replace(currencyNode->GetChildren().front(),
    make_unique<ReferenceNode>("<security"))->SetVisible(false);
  AddChild(BaseSingleOrderTaskFactory::CURRENCY, std::move(currencyNode));
  AddChild(BaseSingleOrderTaskFactory::TIME_IN_FORCE,
    make_unique<TimeInForceNode>());
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

unique_ptr<SingleOrderTaskNode> SingleOrderTaskNode::AddVolumeNode() const {
  auto clone = CanvasNode::Clone(*this);
  if(!clone->FindChild(VOLUME_PROPERTY).is_initialized()) {
    clone->AddChild(VOLUME_PROPERTY, BuildVolumeNode());
  }
  return clone;
}

unique_ptr<SingleOrderTaskNode> SingleOrderTaskNode::RemoveVolumeNode() const {
  auto clone = CanvasNode::Clone(*this);
  auto child = clone->FindChild(VOLUME_PROPERTY);
  if(child.is_initialized()) {
    clone->RemoveChild(*child);
  }
  return clone;
}

unique_ptr<CanvasNode> SingleOrderTaskNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  if(&child == &*FindChild(BaseSingleOrderTaskFactory::DESTINATION) &&
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
