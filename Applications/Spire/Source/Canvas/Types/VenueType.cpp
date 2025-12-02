#include "Spire/Canvas/Types/VenueType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const VenueType& VenueType::GetInstance() {
  static auto instance = std::shared_ptr<VenueType>(new VenueType());
  return *instance;
}

string VenueType::GetName() const {
  return "Venue";
}

const type_info& VenueType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility VenueType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void VenueType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
