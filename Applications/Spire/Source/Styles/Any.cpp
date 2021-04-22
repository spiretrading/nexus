#include "Spire/Styles/Any.hpp"

using namespace Spire;
using namespace Spire::Styles;

bool Any::operator ==(const Any& selector) const {
  return true;
}

bool Any::operator !=(const Any& selector) const {
  return !(*this == selector);
}

std::vector<Stylist*> Spire::Styles::select(
    const Any& selector, Stylist& source) {
  return std::vector{&source};
}
