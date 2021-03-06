#include "Spire/Styles/OrSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

OrSelector::OrSelector(Selector left, Selector right)
  : m_left(std::move(left)),
    m_right(std::move(right)) {}

const Selector& OrSelector::get_left() const {
  return m_left;
}

const Selector& OrSelector::get_right() const {
  return m_right;
}
