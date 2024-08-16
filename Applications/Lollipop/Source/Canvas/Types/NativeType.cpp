#include "Spire/Canvas/Types/NativeType.hpp"

using namespace Spire;

NativeType::operator std::shared_ptr<NativeType> () const {
  std::shared_ptr<CanvasType> instance = *this;
  return std::static_pointer_cast<NativeType>(instance);
}
