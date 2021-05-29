#include "Spire/Styles/PropertyMatchSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

PropertyMatchSelector::PropertyMatchSelector(Property property)
  : m_property(std::move(property)) {}

const Property& PropertyMatchSelector::get_property() const {
  return m_property;
}

bool PropertyMatchSelector::operator ==(
    const PropertyMatchSelector& selector) const {
  return m_property == selector.m_property;
}

bool PropertyMatchSelector::operator !=(
    const PropertyMatchSelector& selector) const {
  return !(*this == selector);
}

PropertyMatchSelector Spire::Styles::matches(Property property) {
  return PropertyMatchSelector(std::move(property));
}

std::unordered_set<Stylist*>
    Spire::Styles::select(const PropertyMatchSelector& selector,
      std::unordered_set<Stylist*> sources) {
  for(auto i = sources.begin(); i != sources.end();) {
    auto& source = **i;
    auto block = source.compute_style();
    auto is_missing_property = true;
    for(auto& property : block) {
      if(property == selector.get_property()) {
        is_missing_property = false;
        break;
      }
    }
    if(is_missing_property) {
      i = sources.erase(i);
    } else {
      ++i;
    }
  }
  return sources;
}
