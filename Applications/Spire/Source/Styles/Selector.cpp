#include "Spire/Styles/Selector.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::type_index Selector::get_type() const {
  return m_selector.type();
}

bool Selector::is_match(const Selector& selector) const {
  return m_matcher(*this, selector);
}
