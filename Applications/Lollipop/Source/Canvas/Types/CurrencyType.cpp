#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const CurrencyType& CurrencyType::GetInstance() {
  static auto instance = std::shared_ptr<CurrencyType>(new CurrencyType());
  return *instance;
}

string CurrencyType::GetName() const {
  return "Currency";
}

const type_info& CurrencyType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility CurrencyType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void CurrencyType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
