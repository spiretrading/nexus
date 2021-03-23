#include "Spire/Styles/ParentSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

ParentSelector::ParentSelector(Selector base, Selector parent)
  : m_base(std::move(base)),
    m_parent(std::move(parent)) {}

const Selector& ParentSelector::get_base() const {
  return m_base;
}

const Selector& ParentSelector::get_parent() const {
  return m_parent;
}

bool ParentSelector::is_match(const ParentSelector& selector) const {
  return m_base.is_match(selector.get_base()) &&
    m_parent.is_match(selector.get_parent());
}
