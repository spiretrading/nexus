#include "Spire/Styles/Any.hpp"

using namespace Spire;
using namespace Spire::Styles;

SelectConnection Spire::Styles::select(const Any& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update) {
  on_update({&base}, {});
  return {};
}

std::size_t std::hash<Any>::operator ()(const Any& selector) const noexcept {
  return 0x9e3779b97f4a7c15;
}
