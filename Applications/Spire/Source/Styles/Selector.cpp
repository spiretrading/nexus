#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/AndSelector.hpp"
#include "Spire/Styles/DescendantSelector.hpp"
#include "Spire/Styles/NotSelector.hpp"
#include "Spire/Styles/OrSelector.hpp"

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

Selector::Selector(AndSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([this] (const Selector& selector) {
      if(selector.get_type() != typeid(AndSelector)) {
        return false;
      }
      auto& left = as<AndSelector>();
      auto& right = selector.as<AndSelector>();
      return left.get_left().is_match(right.get_left()) &&
        left.get_right().is_match(right.get_right());
    }) {}

Selector::Selector(OrSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([this] (const Selector& selector) {
      if(selector.get_type() != typeid(OrSelector)) {
        return false;
      }
      auto& left = as<OrSelector>();
      auto& right = selector.as<OrSelector>();
      return left.get_left().is_match(right.get_left()) &&
        left.get_right().is_match(right.get_right());
    }) {}

Selector::Selector(DescendantSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([this] (const Selector& selector) {
      if(selector.get_type() != typeid(DescendantSelector)) {
        return false;
      }
      auto& left = as<DescendantSelector>();
      auto& right = selector.as<DescendantSelector>();
      return left.get_ancestor().is_match(right.get_ancestor()) &&
        left.get_descendant().is_match(right.get_descendant());
    }) {}

std::type_index Selector::get_type() const {
  return m_selector.type();
}

bool Selector::is_match(const Selector& selector) const {
  return m_matcher(selector);
}
