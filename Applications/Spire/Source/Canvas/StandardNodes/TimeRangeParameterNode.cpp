#include "Spire/Canvas/StandardNodes/TimeRangeParameterNode.hpp"
#include <Beam/TimeService/TimeClientBox.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  string GetDisplayText(const TimeRangeParameterNode& node) {
    return "Time Range";
  }
}

TimeRangeParameterNode::TimeRangeParameterNode() {
  m_startTime.m_specialValue = Queries::Sequence::Present();
  m_endTime.m_specialValue = Queries::Sequence::Last();
  SetText(GetDisplayText(*this));
  SetType(TimeRangeType::GetInstance());
}

TimeRangeParameterNode::TimeRangeParameterNode(
    const TimeRangeParameter& startTime, const TimeRangeParameter& endTime)
    : m_startTime(startTime),
      m_endTime(endTime) {
  SetText(GetDisplayText(*this));
  SetType(TimeRangeType::GetInstance());
}

const TimeRangeParameter& TimeRangeParameterNode::GetStartTime() const {
  return m_startTime;
}

const TimeRangeParameter& TimeRangeParameterNode::GetEndTime() const {
  return m_endTime;
}

unique_ptr<TimeRangeParameterNode> TimeRangeParameterNode::SetTimeRange(
    const TimeRangeParameter& startTime,
    const TimeRangeParameter& endTime) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_startTime = startTime;
  clone->m_endTime = endTime;
  clone->SetText(GetDisplayText(*clone));
  return clone;
}

Range TimeRangeParameterNode::GetTimeRangeQuery(
    TimeClientBox& timeClient) const {
  Range::Point start;
  if(m_startTime.m_offset.is_initialized()) {
    start = timeClient.GetTime() - *m_startTime.m_offset;
  } else if(m_startTime.m_timeOfDay.is_initialized()) {
    start = ptime(gregorian::day_clock::universal_day(),
      *m_startTime.m_timeOfDay);
  } else {
    start = Beam::Queries::Sequence::Present();
  }
  Range::Point end;
  if(m_endTime.m_offset.is_initialized()) {
    end = timeClient.GetTime() - *m_endTime.m_offset;
  } else if(m_endTime.m_timeOfDay.is_initialized()) {
    end = ptime(gregorian::day_clock::universal_day(), *m_endTime.m_timeOfDay);
  } else {
    end = Beam::Queries::Sequence::Last();
  }
  Range timeRange(start, end);
  return timeRange;
}

void TimeRangeParameterNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TimeRangeParameterNode::Clone() const {
  return make_unique<TimeRangeParameterNode>(*this);
}

unique_ptr<CanvasNode> TimeRangeParameterNode::Reset() const {
  return make_unique<TimeRangeParameterNode>();
}

TimeRangeParameterNode::TimeRangeParameterNode(ReceiveBuilder) {}
