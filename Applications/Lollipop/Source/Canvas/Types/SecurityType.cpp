#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const SecurityType& SecurityType::GetInstance() {
  static auto instance = std::shared_ptr<SecurityType>(new SecurityType());
  return *instance;
}

string SecurityType::GetName() const {
  return "Security";
}

const type_info& SecurityType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility SecurityType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void SecurityType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
