#include "Spire/Styles/AncestorSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

AncestorSelector::AncestorSelector(Selector base, Selector ancestor)
  : m_base(std::move(base)),
    m_ancestor(std::move(ancestor)) {}

const Selector& AncestorSelector::get_base() const {
  return m_base;
}

const Selector& AncestorSelector::get_ancestor() const {
  return m_ancestor;
}
