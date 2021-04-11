#include "Spire/Styles/PseudoElement.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::type_index PseudoElement::get_type() const {
  return m_pseudo_element.type();
}

bool PseudoElement::operator ==(const PseudoElement& element) const {
  return m_is_equal(*this, element);
}

bool PseudoElement::operator !=(const PseudoElement& element) const {
  return !(*this == element);
}

std::vector<Stylist*> Spire::Styles::select(
    const PseudoElement& selector, Stylist& source) {
  return selector.m_select(selector, source);
}

std::size_t Spire::Styles::hash_value(const PseudoElement& element) {
  return std::hash<std::type_index>()(element.get_type());
}
