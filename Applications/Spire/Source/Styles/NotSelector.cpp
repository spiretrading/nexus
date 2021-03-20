#include "Spire/Styles/NotSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

NotSelector::NotSelector(Selector selector)
  : m_selector(std::move(selector)) {}

const Selector& NotSelector::get_selector() const {
  return m_selector;
}

bool NotSelector::is_match(const NotSelector& selector) const {
  return m_selector.is_match(selector.get_selector());
}

NotSelector Spire::Styles::operator !(Selector selector) {
  return NotSelector(std::move(selector));
}
