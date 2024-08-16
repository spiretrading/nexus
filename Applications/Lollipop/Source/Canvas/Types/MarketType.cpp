#include "Spire/Canvas/Types/MarketType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const MarketType& MarketType::GetInstance() {
  static auto instance = std::shared_ptr<MarketType>(new MarketType());
  return *instance;
}

string MarketType::GetName() const {
  return "Market";
}

const type_info& MarketType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility MarketType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void MarketType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
