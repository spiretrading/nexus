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

SelectConnection Spire::Styles::select(const PseudoElement& element,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return {};
}

std::size_t Spire::Styles::hash_value(const PseudoElement& element) {
  return std::hash<std::type_index>()(element.get_type());
}

std::size_t std::hash<PseudoElement>::operator
    ()(const PseudoElement& element) const {
  return hash_value(element);
}
