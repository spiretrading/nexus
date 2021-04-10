#include "Spire/Styles/PropertyMatchSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

PropertyMatchSelector::PropertyMatchSelector(Property property)
  : m_property(std::move(property)) {}

const Property& PropertyMatchSelector::get_property() const {
  return m_property;
}

bool PropertyMatchSelector::is_match(
    const PropertyMatchSelector& selector) const {
  return m_property == selector.m_property;
}

PropertyMatchSelector Spire::Styles::matches(Property property) {
  return PropertyMatchSelector(std::move(property));
}

std::vector<QWidget*> Spire::Styles::select(
    const PropertyMatchSelector& selector, QWidget& source) {
  auto block = find_stylist(source).compute_style();
  for(auto& property : block.get_properties()) {
    if(property == selector.get_property()) {
      return {&source};
    }
  }
  return {};
}
