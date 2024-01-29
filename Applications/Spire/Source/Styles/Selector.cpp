#include "Spire/Styles/Selector.hpp"
#include <boost/functional/hash.hpp>

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

std::unordered_map<std::type_index, Selector::Operations>
  Selector::m_operations;

std::type_index Selector::get_type() const {
  return m_selector.type();
}

bool Selector::operator ==(const Selector& selector) const {
  auto& operations = m_operations.at(m_selector.type());
  return operations.m_is_equal(*this, selector);
}

bool Selector::operator !=(const Selector& selector) const {
  return !(*this == selector);
}

SelectConnection Spire::Styles::select(const Selector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  auto& operations = Selector::m_operations.at(selector.get_type());
  return operations.m_select(selector, base, on_update);
}

std::size_t std::hash<Selector>::operator ()(const Selector& selector) const {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<std::type_index>()(selector.get_type()));
  auto& operations = Selector::m_operations.at(selector.get_type());
  hash_combine(seed, operations.m_hash(selector));
  return seed;
}
