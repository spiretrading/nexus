#include "Spire/Styles/IsASelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

const std::type_index& IsASelector::get_type() const {
  return m_type;
}

bool IsASelector::is_instance(const QWidget& widget) const {
  return m_is_instance(widget);
}

bool IsASelector::operator ==(const IsASelector& selector) const {
  return get_type() == selector.get_type();
}

bool IsASelector::operator !=(const IsASelector& selector) const {
  return !(*this == selector);
}

SelectConnection Spire::Styles::select(const IsASelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  if(!base.get_pseudo_element() && selector.is_instance(base.get_widget())) {
    on_update({&base}, {});
  }
  return {};
}