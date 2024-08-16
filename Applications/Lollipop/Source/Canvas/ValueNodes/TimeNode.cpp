#include "Spire/Canvas/ValueNodes/TimeNode.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <QTimeEdit>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace std;

namespace {
  string GetDisplayText(const time_duration& value) {
    QTime timeDisplay(0, 0, 0, 0);
    auto localTime = ToLocalTime(value);
    timeDisplay = timeDisplay.addMSecs(
      static_cast<int>(localTime.total_milliseconds()));
    return timeDisplay.toString("hh:mm:ss.zzz").toStdString();
  }
}

TimeNode::TimeNode()
    : ValueNode(ToUtcTime(seconds(0))) {
  SetText(GetDisplayText(GetValue()));
}

TimeNode::TimeNode(time_duration value)
    : ValueNode(value) {
  SetText(GetDisplayText(GetValue()));
}

unique_ptr<TimeNode> TimeNode::SetValue(time_duration value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void TimeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TimeNode::Clone() const {
  return make_unique<TimeNode>(*this);
}

unique_ptr<CanvasNode> TimeNode::Reset() const {
  return make_unique<TimeNode>();
}
