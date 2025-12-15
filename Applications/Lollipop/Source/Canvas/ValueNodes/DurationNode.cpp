#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include <QTimeEdit>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace std;

namespace {
  string GetDisplayText(const time_duration& value) {
    QTime timeDisplay(0, 0, 0, 0);
    timeDisplay = timeDisplay.addMSecs(
      static_cast<int>(value.total_milliseconds()));
    return timeDisplay.toString("hh:mm:ss.zzz").toStdString();
  }
}

DurationNode::DurationNode()
    : ValueNode(seconds(0)) {
  SetText(GetDisplayText(GetValue()));
}

DurationNode::DurationNode(time_duration value)
    : ValueNode(value) {
  SetText(GetDisplayText(GetValue()));
}

unique_ptr<DurationNode> DurationNode::SetValue(time_duration value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void DurationNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> DurationNode::Clone() const {
  return std::make_unique<DurationNode>(*this);
}

std::unique_ptr<CanvasNode> DurationNode::Reset() const {
  return std::make_unique<DurationNode>();
}
