#include "Spire/Styles/Any.hpp"

using namespace Spire;
using namespace Spire::Styles;

bool Any::operator ==(const Any& selector) const {
  return true;
}

bool Any::operator !=(const Any& selector) const {
  return !(*this == selector);
}

SelectConnection Spire::Styles::select(const Any& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update) {
  on_update({&base}, {});
  return {};
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const Any& selector, std::unordered_set<Stylist*> sources) {
  return sources;
}
