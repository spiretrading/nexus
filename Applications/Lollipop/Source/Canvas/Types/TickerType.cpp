#include "Spire/Canvas/Types/TickerType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const TickerType& TickerType::GetInstance() {
  static auto instance = std::shared_ptr<TickerType>(new TickerType());
  return *instance;
}

string TickerType::GetName() const {
  return "Ticker";
}

const type_info& TickerType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility TickerType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void TickerType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
