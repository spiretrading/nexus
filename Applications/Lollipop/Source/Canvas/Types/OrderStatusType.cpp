#include "Spire/Canvas/Types/OrderStatusType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const OrderStatusType& OrderStatusType::GetInstance() {
  static auto instance = std::shared_ptr<OrderStatusType>(
    new OrderStatusType());
  return *instance;
}

string OrderStatusType::GetName() const {
  return "Order Status";
}

const type_info& OrderStatusType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility OrderStatusType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void OrderStatusType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
