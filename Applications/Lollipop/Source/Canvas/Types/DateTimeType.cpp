#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const DateTimeType& DateTimeType::GetInstance() {
  static auto instance = std::shared_ptr<DateTimeType>(new DateTimeType());
  return *instance;
}

string DateTimeType::GetName() const {
  return "Date/Time";
}

const type_info& DateTimeType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility DateTimeType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void DateTimeType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
