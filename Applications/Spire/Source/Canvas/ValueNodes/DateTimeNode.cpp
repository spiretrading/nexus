#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::date_time;
using namespace boost::posix_time;
using namespace Spire;

namespace {
  std::string GetDisplayText(ptime value) {
    if(value.is_special() || value.is_not_a_date_time()) {
      return to_simple_string(value);
    } else {
      return to_simple_string(ToLocalTime(value));
    }
  }
}

DateTimeNode::DateTimeNode() {
  SetText(GetDisplayText(GetValue()));
}

DateTimeNode::DateTimeNode(const ptime& value)
    : ValueNode<DateTimeType>(value) {
  SetText(GetDisplayText(GetValue()));
}

std::unique_ptr<DateTimeNode> DateTimeNode::SetValue(const ptime& value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(value);
  clone->SetText(GetDisplayText(clone->GetValue()));
  return clone;
}

void DateTimeNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> DateTimeNode::Clone() const {
  return std::make_unique<DateTimeNode>(*this);
}

std::unique_ptr<CanvasNode> DateTimeNode::Reset() const {
  return std::make_unique<DateTimeNode>();
}
