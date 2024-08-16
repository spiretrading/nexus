#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const DurationType& DurationType::GetInstance() {
  static auto instance = std::shared_ptr<DurationType>(new DurationType());
  return *instance;
}

string DurationType::GetName() const {
  return "Duration";
}

const type_info& DurationType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility DurationType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void DurationType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
