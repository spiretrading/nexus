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

bool OrSelector::operator ==(const OrSelector& selector) const {
  return m_left == selector.get_left() && m_right == selector.get_right();
}

bool OrSelector::operator !=(const OrSelector& selector) const {
  return !(*this == selector);
}

OrSelector Spire::Styles::operator ||(Selector left, Selector right) {
  return OrSelector(std::move(left), std::move(right));
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const OrSelector& selector, std::unordered_set<Stylist*> sources) {
  auto left = select(selector.get_left(), sources);
  if(left.empty()) {
    return select(selector.get_right(), std::move(sources));
  }
  auto right = select(selector.get_right(), std::move(sources));
  if(right.empty()) {
    return left;
  }
  left.insert(right.begin(), right.end());
  return left;
}
