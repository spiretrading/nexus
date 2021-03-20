#include "Spire/Styles/SiblingSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

SiblingSelector::SiblingSelector(Selector base, Selector sibling)
  : m_base(std::move(base)),
    m_sibling(std::move(sibling)) {}

const Selector& SiblingSelector::get_base() const {
  return m_base;
}

const Selector& SiblingSelector::get_sibling() const {
  return m_sibling;
}

bool SiblingSelector::is_match(const SiblingSelector& selector) const {
  return m_base.is_match(selector.get_base()) &&
    m_sibling.is_match(selector.get_sibling());
}

SiblingSelector Spire::Styles::operator %(Selector base, Selector sibling) {
  return SiblingSelector(std::move(base), std::move(sibling));
}
