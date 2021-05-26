#include "Spire/Styles/PathSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

PathSelector::PathSelector(Selector first, Selector second)
  : m_first(std::move(first)),
    m_second(std::move(second)) {}

const Selector& PathSelector::get_first() const {
  return m_first;
}

const Selector& PathSelector::get_second() const {
  return m_second;
}

bool PathSelector::operator ==(const PathSelector& selector) const {
  return m_first == selector.get_first() && m_second == selector.get_second();
}

bool PathSelector::operator !=(const PathSelector& selector) const {
  return !(*this == selector);
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const PathSelector& selector, std::unordered_set<Stylist*> sources) {
  sources = select(selector.get_first(), std::move(sources));
  sources = select(selector.get_second(), std::move(sources));
  return sources;
}

std::vector<QWidget*> Spire::Styles::build_reach(
    const PathSelector& selector, QWidget& source) {
  auto reach = std::unordered_set<QWidget*>();
  auto first = build_reach(selector.get_first(), source);
  reach.insert(first.begin(), first.end());
  auto second = build_reach(selector.get_second(), source);
  reach.insert(second.begin(), second.end());
  return std::vector(reach.begin(), reach.end());
}
