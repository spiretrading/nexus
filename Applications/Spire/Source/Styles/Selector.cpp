#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/NotSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

Selector::Selector(Any any)
  : m_selector(std::move(any)),
    m_matcher([] (const Selector& selector) {
      return selector.get_type() == typeid(Any);
    }) {}

Selector::Selector(NotSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([this] (const Selector& selector) {
      if(selector.get_type() != typeid(NotSelector)) {
        return false;
      }
      auto& left = as<NotSelector>();
      auto& right = selector.as<NotSelector>();
      return left.get_selector().is_match(right.get_selector());
    }) {}

std::type_index Selector::get_type() const {
  return m_selector.type();
}

bool Selector::is_match(const Selector& selector) const {
  return m_matcher(selector);
}
