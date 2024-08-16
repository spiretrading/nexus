#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const SideType& SideType::GetInstance() {
  static auto instance = std::shared_ptr<SideType>(new SideType());
  return *instance;
}

string SideType::GetName() const {
  return "Side";
}

const type_info& SideType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility SideType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void SideType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
