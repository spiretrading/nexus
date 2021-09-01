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

SelectConnection Spire::Styles::select(const PropertyMatchSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return {};
}
