#include "Spire/Styles/NotSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

NotSelector::NotSelector(Selector selector)
  : m_selector(std::move(selector)) {}

const Selector& NotSelector::get_selector() const {
  return m_selector;
}

bool NotSelector::operator ==(const NotSelector& selector) const {
  return m_selector == selector.get_selector();
}

bool NotSelector::operator !=(const NotSelector& selector) const {
  return !(*this == selector);
}

NotSelector Spire::Styles::operator !(Selector selector) {
  return NotSelector(std::move(selector));
}

SelectConnection Spire::Styles::select(const NotSelector& selector,
    const Stylist& base, const SelectionUpdate& on_update) {
  return select(selector.get_selector(), base,
    [=] (std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      on_update(std::move(removals), std::move(additions));
    });
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const NotSelector& selector, std::unordered_set<Stylist*> sources) {
  for(auto selected : select(selector.get_selector(), sources)) {
    sources.erase(selected);
  }
  return sources;
}
