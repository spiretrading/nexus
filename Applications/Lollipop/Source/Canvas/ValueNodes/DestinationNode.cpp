#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/TickerType.hpp"
#include "Spire/Canvas/Types/VenueType.hpp"

using namespace Beam;
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

Venue DestinationNode::GetVenue() const {
  if(m_referent.empty()) {
    return Venue();
  }
  auto name = m_referent;
  boost::optional<const CanvasNode&> node = *this;
  while(node.is_initialized() && !name.empty() && name[0] == '<') {
    if(IsRoot(*node)) {
      return Venue();
    }
    node = node->GetParent();
    name = name.substr(1);
  }
  if(!node.is_initialized()) {
    return Venue();
  }
  if(name.empty()) {
    if(auto tickerNode = dynamic_cast<const ValueNode<TickerType>*>(
        &*node)) {
      return tickerNode->GetValue().get_venue();
    }
    if(auto venueNode = dynamic_cast<const ValueNode<VenueType>*>(&*node)) {
      return venueNode->GetValue();
    }
    return Venue();
  }
  if(IsRoot(*node)) {
    return Venue();
  }
  node = node->GetParent()->FindNode(name);
  if(!node.is_initialized()) {
    return Venue();
  }
  if(auto tickerNode = dynamic_cast<const ValueNode<TickerType>*>(&*node)) {
    return tickerNode->GetValue().get_venue();
  }
  if(auto venueNode = dynamic_cast<const ValueNode<VenueType>*>(&*node)) {
    return venueNode->GetValue();
  }
  return Venue();
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
  return std::make_unique<DestinationNode>(*this);
}

unique_ptr<CanvasNode> DestinationNode::Reset() const {
  return make_unique<DestinationNode>();
}
