#include "Spire/Canvas/ValueNodes/TimeRangeNode.hpp"
#include <Beam/Queries/Sequence.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::date_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  string GetDisplayText(const Range& value) {
    if(value == Range::REAL_TIME) {
      return "Real-Time";
    }
    string startingPoint;
    if(value.get_start() == Beam::Sequence::PRESENT) {
      startingPoint = "Present";
    } else {
      startingPoint = to_simple_string(to_local_time(boost::get<ptime>(
        value.get_start())));
    }
    string endingPoint;
    if(value.get_end() == Beam::Sequence::PRESENT) {
      endingPoint = "Present";
    } else {
      endingPoint = to_simple_string(to_local_time(boost::get<ptime>(
        value.get_end())));
    }
    return startingPoint + " -> " + endingPoint;
  }
}

TimeRangeNode::TimeRangeNode() {
  SetText(GetDisplayText(GetValue()));
}

TimeRangeNode::TimeRangeNode(const Range& value)
    : ValueNode<TimeRangeType>(value) {
  SetText(GetDisplayText(GetValue()));
}

unique_ptr<TimeRangeNode> TimeRangeNode::SetValue(const Range& value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void TimeRangeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TimeRangeNode::Clone() const {
  return std::make_unique<TimeRangeNode>(*this);
}

unique_ptr<CanvasNode> TimeRangeNode::Reset() const {
  return make_unique<TimeRangeNode>();
}
