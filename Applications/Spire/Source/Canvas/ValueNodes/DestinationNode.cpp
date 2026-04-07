#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/VenueType.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

DestinationNode::DestinationNode() {
  SetText(GetValue());
}

DestinationNode::DestinationNode(std::string value)
    : ValueNode<DestinationType>(std::move(value)) {
  SetText(GetValue());
}

std::unique_ptr<DestinationNode>
    DestinationNode::SetValue(std::string value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(std::move(value));
  clone->SetText(clone->GetValue());
  return clone;
}

Venue DestinationNode::GetVenue() const {
  if(m_referent.empty()) {
    return Venue();
  }
  auto name = m_referent;
  auto node = optional<const CanvasNode&>(*this);
  while(node && !name.empty() && name[0] == '<') {
    if(IsRoot(*node)) {
      return Venue();
    }
    node = node->GetParent();
    name = name.substr(1);
  }
  if(!node) {
    return Venue();
  }
  if(name.empty()) {
    if(auto securityNode = dynamic_cast<const ValueNode<SecurityType>*>(
        &*node)) {
      return securityNode->GetValue().get_venue();
    }
    if(auto marketNode = dynamic_cast<const ValueNode<VenueType>*>(&*node)) {
      return marketNode->GetValue();
    }
    return Venue();
  }
  if(IsRoot(*node)) {
    return Venue();
  }
  node = node->GetParent()->FindNode(name);
  if(!node) {
    return Venue();
  }
  if(auto securityNode = dynamic_cast<const ValueNode<SecurityType>*>(&*node)) {
    return securityNode->GetValue().get_venue();
  }
  if(auto marketNode = dynamic_cast<const ValueNode<VenueType>*>(&*node)) {
    return marketNode->GetValue();
  }
  return Venue();
}

void DestinationNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

const std::string& DestinationNode::GetReferent() const {
  return m_referent;
}

std::unique_ptr<CanvasNode>
    DestinationNode::SetReferent(const std::string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  return clone;
}

std::unique_ptr<CanvasNode> DestinationNode::Clone() const {
  return std::make_unique<DestinationNode>(*this);
}

std::unique_ptr<CanvasNode> DestinationNode::Reset() const {
  return std::make_unique<DestinationNode>();
}
