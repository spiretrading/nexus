#include "Spire/Styles/Selector.hpp"

using namespace Spire;
using namespace Spire::Styles;

Selector::Selector(Any any)
  : m_selector(std::move(any)),
    m_matcher([] (const Selector& selector) {
      return selector.get_type() == typeid(Any);
    }) {}

std::type_index Selector::get_type() const {
  return m_selector.type();
}

bool Selector::is_match(const Selector& selector) const {
  return m_matcher(selector);
}
