#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const BooleanType& BooleanType::GetInstance() {
  static auto instance = std::shared_ptr<BooleanType>(new BooleanType());
  return *instance;
}

string BooleanType::GetName() const {
  return "Boolean";
}

const type_info& BooleanType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility BooleanType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void BooleanType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
