#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

TimeInForceNode::TimeInForceNode()
    : ValueNode(TimeInForce(TimeInForce::Type::DAY)) {
  SetText(lexical_cast<string>(GetValue().GetType()));
}

TimeInForceNode::TimeInForceNode(const TimeInForce& value)
    : ValueNode(value) {
  SetText(lexical_cast<string>(GetValue().GetType()));
}

unique_ptr<TimeInForceNode> TimeInForceNode::SetValue(
    const TimeInForce& value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(lexical_cast<string>(clone->GetValue().GetType()));
  return clone;
}

void TimeInForceNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TimeInForceNode::Clone() const {
  return make_unique<TimeInForceNode>(*this);
}

unique_ptr<CanvasNode> TimeInForceNode::Reset() const {
  return make_unique<TimeInForceNode>();
}
