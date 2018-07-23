#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const MoneyType& MoneyType::GetInstance() {
  static auto instance = std::shared_ptr<MoneyType>(new MoneyType());
  return *instance;
}

string MoneyType::GetName() const {
  return "Money";
}

const type_info& MoneyType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility MoneyType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void MoneyType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
