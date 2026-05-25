#include "Spire/Styles/PseudoElement.hpp"
#include <boost/functional/hash.hpp>
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

std::unordered_map<std::type_index, PseudoElement::Operations>
  PseudoElement::m_operations;

PseudoElement::Operations::Operations(
  bool (*is_equal)(const PseudoElement&, const PseudoElement&),
  SelectConnection (*select)(
    const PseudoElement&, const Stylist&, const SelectionUpdateSignal&),
  std::size_t (*hash)(const PseudoElement&)) noexcept
  : m_is_equal(is_equal),
    m_select(select),
    m_hash(hash) {}

SelectConnection Spire::Styles::Details::select_pseudo_element(
    const PseudoElement& selector, const Stylist& base,
    const SelectionUpdateSignal& on_update) {
  if(auto pseudo_stylist = find_stylist(base.get_widget(), selector)) {
    on_update({pseudo_stylist}, {});
  }
  return {};
}

std::type_index PseudoElement::get_type() const {
  return m_holder->get_type();
}

bool PseudoElement::operator ==(const PseudoElement& element) const {
  auto& operations = m_operations.at(m_holder->get_type());
  return operations.m_is_equal(*this, element);
}

SelectConnection Spire::Styles::select(const PseudoElement& element,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  auto& operations = PseudoElement::m_operations.at(element.get_type());
  return operations.m_select(element, base, on_update);
}

std::size_t std::hash<PseudoElement>::operator ()(
    const PseudoElement& element) const noexcept {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<std::type_index>()(element.get_type()));
  auto& operations = PseudoElement::m_operations.at(element.get_type());
  hash_combine(seed, operations.m_hash(element));
  return seed;
}
