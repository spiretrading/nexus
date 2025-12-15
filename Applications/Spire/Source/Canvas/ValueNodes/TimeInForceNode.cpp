#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

TimeInForceNode::TimeInForceNode()
    : ValueNode(TimeInForce(TimeInForce::Type::DAY)) {
  SetText(lexical_cast<std::string>(GetValue().get_type()));
}

TimeInForceNode::TimeInForceNode(const TimeInForce& value)
    : ValueNode(value) {
  SetText(lexical_cast<std::string>(GetValue().get_type()));
}

std::unique_ptr<TimeInForceNode> TimeInForceNode::SetValue(
    const TimeInForce& value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(lexical_cast<std::string>(clone->GetValue().get_type()));
  return clone;
}

void TimeInForceNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> TimeInForceNode::Clone() const {
  return std::make_unique<TimeInForceNode>(*this);
}

std::unique_ptr<CanvasNode> TimeInForceNode::Reset() const {
  return std::make_unique<TimeInForceNode>();
}
