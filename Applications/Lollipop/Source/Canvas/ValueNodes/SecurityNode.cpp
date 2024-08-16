#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

SecurityNode::SecurityNode() {
  SetText("");
}

SecurityNode::SecurityNode(const Security& value,
    const MarketDatabase& marketDatabase)
    : ValueNode(value) {
  SetText(ToString(value, marketDatabase));
}

unique_ptr<SecurityNode> SecurityNode::SetValue(const Security& value,
    const MarketDatabase& marketDatabase) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(ToString(clone->GetValue(), marketDatabase));
  return clone;
}

void SecurityNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> SecurityNode::Clone() const {
  return make_unique<SecurityNode>(*this);
}

unique_ptr<CanvasNode> SecurityNode::Reset() const {
  return make_unique<SecurityNode>();
}
