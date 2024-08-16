#include "Spire/Canvas/Types/TimeInForceType.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const TimeInForceType& TimeInForceType::GetInstance() {
  static auto instance = std::shared_ptr<TimeInForceType>(
    new TimeInForceType());
  return *instance;
}

string TimeInForceType::GetName() const {
  return "Time In Force";
}

const type_info& TimeInForceType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility TimeInForceType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void TimeInForceType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
