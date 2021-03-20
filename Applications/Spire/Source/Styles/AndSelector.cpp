#include "Spire/Styles/AndSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

AndSelector::AndSelector(Selector left, Selector right)
  : m_left(std::move(left)),
    m_right(std::move(right)) {}

const Selector& AndSelector::get_left() const {
  return m_left;
}

const Selector& AndSelector::get_right() const {
  return m_right;
}

bool AndSelector::is_match(const AndSelector& selector) const {
  return m_left.is_match(selector.get_left()) &&
    m_right.is_match(selector.get_right());
}

AndSelector Spire::Styles::operator &&(Selector left, Selector right) {
  return AndSelector(std::move(left), std::move(right));
}
