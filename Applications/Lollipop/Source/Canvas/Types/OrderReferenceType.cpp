#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const OrderReferenceType& OrderReferenceType::GetInstance() {
  static auto instance = std::shared_ptr<OrderReferenceType>(
    new OrderReferenceType());
  return *instance;
}

string OrderReferenceType::GetName() const {
  return "Order";
}

const type_info& OrderReferenceType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility OrderReferenceType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void OrderReferenceType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
