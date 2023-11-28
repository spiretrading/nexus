#include "Spire/Styles/Any.hpp"

using namespace Spire;
using namespace Spire::Styles;

SelectConnection Spire::Styles::select(const Any& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update) {
  on_update({&base}, {});
  return {};
}

std::size_t std::hash<Any>::operator ()(const Any& selector) {
  return 1;
}
