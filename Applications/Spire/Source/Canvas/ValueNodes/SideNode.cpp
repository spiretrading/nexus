#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  string GetDisplayText(Side value) {
    if(value == Side::BID) {
      return "Bid";
    } else if(value == Side::ASK) {
      return "Ask";
    }
    return "None";
  }
}

SideNode::SideNode()
    : ValueNode(Side::BID) {
  SetText(GetDisplayText(GetValue()));
}

SideNode::SideNode(Side value)
    : ValueNode(value) {
  SetText(GetDisplayText(GetValue()));
}

unique_ptr<SideNode> SideNode::SetValue(Side value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void SideNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> SideNode::Clone() const {
  return make_unique<SideNode>(*this);
}

unique_ptr<CanvasNode> SideNode::Reset() const {
  return make_unique<SideNode>();
}
