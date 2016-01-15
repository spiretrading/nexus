#include "Spire/Canvas/Types/TimeRangeType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const TimeRangeType& TimeRangeType::GetInstance() {
  static auto instance = std::shared_ptr<TimeRangeType>(new TimeRangeType());
  return *instance;
}

string TimeRangeType::GetName() const {
  return "Time Range";
}

const type_info& TimeRangeType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility TimeRangeType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void TimeRangeType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
