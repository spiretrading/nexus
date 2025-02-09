#include "Spire/Canvas/ValueNodes/TimeRangeNode.hpp"
#include <Beam/Queries/Sequence.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::date_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  std::string GetDisplayText(const Range& value) {
    if(value == Range::RealTime()) {
      return "Real-Time";
    }
    auto startingPoint = [&] {
      if(value.GetStart() == Queries::Sequence::Present()) {
        return std::string("Present");
      } else {
        return to_simple_string(
          ToLocalTime(boost::get<ptime>(value.GetStart())));
      }
    }();
    auto endingPoint = [&] {
      if(value.GetStart() == Queries::Sequence::Present()) {
        return std::string("Present");
      } else {
        return to_simple_string(ToLocalTime(boost::get<ptime>(value.GetEnd())));
      }
    }();
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

std::unique_ptr<TimeRangeNode>
    TimeRangeNode::SetValue(const Range& value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void TimeRangeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> TimeRangeNode::Clone() const {
  return std::make_unique<TimeRangeNode>(*this);
}

std::unique_ptr<CanvasNode> TimeRangeNode::Reset() const {
  return std::make_unique<TimeRangeNode>();
}
