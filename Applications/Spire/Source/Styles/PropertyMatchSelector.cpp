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

std::vector<Stylist*> Spire::Styles::select(
    const PropertyMatchSelector& selector, Stylist& source) {
  auto block = source.compute_style();
  for(auto& property : block.get_properties()) {
    if(property == selector.get_property()) {
      return {&source};
    }
  }
  return {};
}
