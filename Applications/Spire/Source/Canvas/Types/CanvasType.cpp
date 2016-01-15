#include "Spire/Canvas/Types/CanvasType.hpp"

using namespace Spire;

CanvasType::operator std::shared_ptr<CanvasType> () const {
  return std::const_pointer_cast<CanvasType>(shared_from_this());
}

bool Spire::IsCompatible(CanvasType::Compatibility compatibility) {
  return compatibility  == CanvasType::Compatibility::COMPATIBLE ||
    compatibility == CanvasType::Compatibility::EQUAL;
}

bool Spire::IsCompatible(const CanvasType& a, const CanvasType& b) {
  auto compatibility = a.GetCompatibility(b);
  return IsCompatible(compatibility);
}
