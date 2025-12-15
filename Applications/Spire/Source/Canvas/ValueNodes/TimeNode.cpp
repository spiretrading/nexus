#include "Spire/Canvas/ValueNodes/TimeNode.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <QTimeEdit>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Spire;

namespace {
  std::string GetDisplayText(const time_duration& value) {
    auto timeDisplay = QTime(0, 0, 0, 0);
    auto localTime = to_local_time(value);
    timeDisplay = timeDisplay.addMSecs(
      static_cast<int>(localTime.total_milliseconds()));
    return timeDisplay.toString("hh:mm:ss.zzz").toStdString();
  }
}

TimeNode::TimeNode()
    : ValueNode(to_utc_time(seconds(0))) {
  SetText(GetDisplayText(GetValue()));
}

TimeNode::TimeNode(time_duration value)
    : ValueNode(value) {
  SetText(GetDisplayText(GetValue()));
}

std::unique_ptr<TimeNode> TimeNode::SetValue(time_duration value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void TimeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> TimeNode::Clone() const {
  return std::make_unique<TimeNode>(*this);
}

std::unique_ptr<CanvasNode> TimeNode::Reset() const {
  return std::make_unique<TimeNode>();
}
