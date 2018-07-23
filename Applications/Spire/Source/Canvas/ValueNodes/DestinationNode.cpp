#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/MarketType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

DestinationNode::DestinationNode() {
  SetText(GetValue());
}

DestinationNode::DestinationNode(string value)
    : ValueNode<DestinationType>(std::move(value)) {
  SetText(GetValue());
}

unique_ptr<DestinationNode> DestinationNode::SetValue(string value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(std::move(value));
  clone->SetText(clone->GetValue());
  return clone;
}

MarketCode DestinationNode::GetMarket() const {
  if(m_referent.empty()) {
    return MarketCode();
  }
  auto name = m_referent;
  boost::optional<const CanvasNode&> node = *this;
  while(node.is_initialized() && !name.empty() && name[0] == '<') {
    if(IsRoot(*node)) {
      return MarketCode();
    }
    node = node->GetParent();
    name = name.substr(1);
  }
  if(!node.is_initialized()) {
    return MarketCode();
  }
  if(name.empty()) {
    if(auto securityNode = dynamic_cast<const ValueNode<SecurityType>*>(
        &*node)) {
      return securityNode->GetValue().GetMarket();
    }
    if(auto marketNode = dynamic_cast<const ValueNode<MarketType>*>(&*node)) {
      return marketNode->GetValue();
    }
    return MarketCode();
  }
  if(IsRoot(*node)) {
    return MarketCode();
  }
  node = node->GetParent()->FindNode(name);
  if(!node.is_initialized()) {
    return MarketCode();
  }
  if(auto securityNode = dynamic_cast<const ValueNode<SecurityType>*>(&*node)) {
    return securityNode->GetValue().GetMarket();
  }
  if(auto marketNode = dynamic_cast<const ValueNode<MarketType>*>(&*node)) {
    return marketNode->GetValue();
  }
  return MarketCode();
}

void DestinationNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

const string& DestinationNode::GetReferent() const {
  return m_referent;
}

unique_ptr<CanvasNode> DestinationNode::SetReferent(
    const string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  return std::move(clone);
}

unique_ptr<CanvasNode> DestinationNode::Clone() const {
  return make_unique<DestinationNode>(*this);
}

unique_ptr<CanvasNode> DestinationNode::Reset() const {
  return make_unique<DestinationNode>();
}

DestinationNode::DestinationNode(ReceiveBuilder) {}
