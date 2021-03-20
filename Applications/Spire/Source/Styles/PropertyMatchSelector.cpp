#include "Spire/Styles/PropertyMatchSelector.hpp"

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
