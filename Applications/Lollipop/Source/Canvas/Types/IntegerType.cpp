#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const IntegerType& IntegerType::GetInstance() {
  static auto instance = std::shared_ptr<IntegerType>(new IntegerType());
  return *instance;
}

string IntegerType::GetName() const {
  return "Integer";
}

const type_info& IntegerType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility IntegerType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void IntegerType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
