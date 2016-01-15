#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const DecimalType& DecimalType::GetInstance() {
  static auto instance = std::shared_ptr<DecimalType>(new DecimalType());
  return *instance;
}

string DecimalType::GetName() const {
  return "Decimal";
}

const type_info& DecimalType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility DecimalType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void DecimalType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
