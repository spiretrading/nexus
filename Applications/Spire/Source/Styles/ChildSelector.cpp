#include "Spire/Styles/ChildSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

ChildSelector::ChildSelector(Selector base, Selector child)
  : m_base(std::move(base)),
    m_child(std::move(child)) {}

const Selector& ChildSelector::get_base() const {
  return m_base;
}

const Selector& ChildSelector::get_child() const {
  return m_child;
}

bool ChildSelector::is_match(const ChildSelector& selector) const {
  return m_base.is_match(selector.get_base()) &&
    m_child.is_match(selector.get_child());
}

ChildSelector Spire::Styles::operator >(Selector base, Selector child) {
  return ChildSelector(std::move(base), std::move(child));
}
