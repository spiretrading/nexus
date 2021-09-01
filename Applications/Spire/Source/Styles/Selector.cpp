#include "Spire/Styles/Selector.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::type_index Selector::get_type() const {
  return m_selector.type();
}

bool Selector::operator ==(const Selector& selector) const {
  return m_is_equal(*this, selector);
}

bool Selector::operator !=(const Selector& selector) const {
  return !(*this == selector);
}

SelectConnection Spire::Styles::select(const Selector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return selector.m_select(selector, base, on_update);
}
