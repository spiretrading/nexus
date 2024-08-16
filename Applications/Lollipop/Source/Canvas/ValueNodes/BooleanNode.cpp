#include "Spire/Canvas/ValueNodes/BooleanNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  string GetDisplayText(bool value) {
    if(value) {
      return "true";
    } else {
      return "false";
    }
  }
}

BooleanNode::BooleanNode()
    : ValueNode<BooleanType>(false) {
  SetText(GetDisplayText(GetValue()));
}

BooleanNode::BooleanNode(bool value)
    : ValueNode<BooleanType>(value) {
  SetText(GetDisplayText(GetValue()));
}

unique_ptr<BooleanNode> BooleanNode::SetValue(bool value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void BooleanNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> BooleanNode::Clone() const {
  return make_unique<BooleanNode>(*this);
}

unique_ptr<CanvasNode> BooleanNode::Reset() const {
  return make_unique<BooleanNode>();
}
