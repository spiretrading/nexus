#include "Spire/Styles/PathSelector.hpp"
#include "Spire/Styles/CombinatorSelector.hpp"

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

SelectConnection Spire::Styles::select(const PathSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_first(), selector.get_second(),
    [] (const Stylist& stylist) {
      return std::unordered_set{&stylist};
    }), base, on_update);
}
