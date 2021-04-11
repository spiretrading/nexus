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

std::vector<Stylist*> Spire::Styles::select(const IsASelector& selector,
    Stylist& source) {
  if(source.get_pseudo_element()) {
    return {};
  }
  if(selector.is_instance(source.get_widget())) {
    return std::vector{&source};
  }
  return {};
}
