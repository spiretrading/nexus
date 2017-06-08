#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const DestinationType& DestinationType::GetInstance() {
  static auto instance = std::shared_ptr<DestinationType>(
    new DestinationType());
  return *instance;
}

string DestinationType::GetName() const {
  return "Destination";
}

const type_info& DestinationType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility DestinationType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void DestinationType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
