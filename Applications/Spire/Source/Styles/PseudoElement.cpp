#include "Spire/Styles/PseudoElement.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

SelectConnection Spire::Styles::Details::select_pseudo_element(
    const PseudoElement& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update) {
  if(auto pseudo_stylist = find_stylist(base.get_widget(), selector)) {
    on_update({pseudo_stylist}, {});
  }
  return {};
}

std::type_index PseudoElement::get_type() const {
  return m_pseudo_element.type();
}

bool PseudoElement::operator ==(const PseudoElement& element) const {
  return m_is_equal(*this, element);
}

bool PseudoElement::operator !=(const PseudoElement& element) const {
  return !(*this == element);
}

SelectConnection Spire::Styles::select(const PseudoElement& element,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return element.m_select(element, base, on_update);
}

std::size_t Spire::Styles::hash_value(const PseudoElement& element) {
  return std::hash<std::type_index>()(element.get_type());
}

std::size_t std::hash<PseudoElement>::operator
    ()(const PseudoElement& element) const {
  return hash_value(element);
}
