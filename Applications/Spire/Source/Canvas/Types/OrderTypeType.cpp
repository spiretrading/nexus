#include "Spire/Canvas/Types/OrderTypeType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const OrderTypeType& OrderTypeType::GetInstance() {
  static auto instance = std::shared_ptr<OrderTypeType>(new OrderTypeType());
  return *instance;
}

string OrderTypeType::GetName() const {
  return "Order Type";
}

const type_info& OrderTypeType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility OrderTypeType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void OrderTypeType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
