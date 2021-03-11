#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/AncestorSelector.hpp"
#include "Spire/Styles/AndSelector.hpp"
#include "Spire/Styles/ChildSelector.hpp"
#include "Spire/Styles/DescendantSelector.hpp"
#include "Spire/Styles/IsASelector.hpp"
#include "Spire/Styles/NotSelector.hpp"
#include "Spire/Styles/OrSelector.hpp"
#include "Spire/Styles/ParentSelector.hpp"
#include "Spire/Styles/VoidSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

Selector::Selector(Any any)
  : m_selector(std::move(any)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      return selector.get_type() == typeid(Any);
    }) {}

Selector::Selector(NotSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      if(selector.get_type() != typeid(NotSelector)) {
        return false;
      }
      auto& left = self.as<NotSelector>();
      auto& right = selector.as<NotSelector>();
      return left.get_selector().is_match(right.get_selector());
    }) {}

Selector::Selector(AndSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      if(selector.get_type() != typeid(AndSelector)) {
        return false;
      }
      auto& left = self.as<AndSelector>();
      auto& right = selector.as<AndSelector>();
      return left.get_left().is_match(right.get_left()) &&
        left.get_right().is_match(right.get_right());
    }) {}

Selector::Selector(OrSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      if(selector.get_type() != typeid(OrSelector)) {
        return false;
      }
      auto& left = self.as<OrSelector>();
      auto& right = selector.as<OrSelector>();
      return left.get_left().is_match(right.get_left()) &&
        left.get_right().is_match(right.get_right());
    }) {}

Selector::Selector(IsASelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      if(selector.get_type() != typeid(IsASelector)) {
        return false;
      }
      auto& left = self.as<IsASelector>();
      auto& right = selector.as<IsASelector>();
      return left.get_type() == right.get_type();
    }) {}

Selector::Selector(AncestorSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      if(selector.get_type() != typeid(AncestorSelector)) {
        return false;
      }
      auto& left = self.as<AncestorSelector>();
      auto& right = selector.as<AncestorSelector>();
      return left.get_base().is_match(right.get_base()) &&
        left.get_ancestor().is_match(right.get_ancestor());
    }) {}

Selector::Selector(ParentSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      if(selector.get_type() != typeid(ParentSelector)) {
        return false;
      }
      auto& left = self.as<ParentSelector>();
      auto& right = selector.as<ParentSelector>();
      return left.get_base().is_match(right.get_base()) &&
        left.get_parent().is_match(right.get_parent());
    }) {}

Selector::Selector(DescendantSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      if(selector.get_type() != typeid(DescendantSelector)) {
        return false;
      }
      auto& left = self.as<DescendantSelector>();
      auto& right = selector.as<DescendantSelector>();
      return left.get_base().is_match(right.get_base()) &&
        left.get_descendant().is_match(right.get_descendant());
    }) {}

Selector::Selector(ChildSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      if(selector.get_type() != typeid(ChildSelector)) {
        return false;
      }
      auto& left = self.as<ChildSelector>();
      auto& right = selector.as<ChildSelector>();
      return left.get_base().is_match(right.get_base()) &&
        left.get_child().is_match(right.get_child());
    }) {}

Selector::Selector(VoidSelector selector)
  : m_selector(std::move(selector)),
    m_matcher([] (const Selector& self, const Selector& selector) {
      return false;
    }) {}

std::type_index Selector::get_type() const {
  return m_selector.type();
}

bool Selector::is_match(const Selector& selector) const {
  return m_matcher(*this, selector);
}
