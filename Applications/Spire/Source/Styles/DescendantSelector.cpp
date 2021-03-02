#include "Spire/Styles/DescendantSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

DescendantSelector::DescendantSelector(Selector ancestor, Selector descendant)
  : m_ancestor(std::move(ancestor)),
    m_descendant(std::move(descendant)) {}

const Selector& DescendantSelector::get_ancestor() const {
  return m_ancestor;
}

const Selector& DescendantSelector::get_descendant() const {
  return m_descendant;
}
