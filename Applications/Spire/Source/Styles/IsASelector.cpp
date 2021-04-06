#include "Spire/Styles/IsASelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

const std::type_index& IsASelector::get_type() const {
  return m_type;
}

bool IsASelector::is_instance(const QWidget& widget) const {
  return m_is_instance(widget);
}

bool IsASelector::is_match(const IsASelector& selector) const {
  return get_type() == selector.get_type();
}

std::vector<QWidget*> Spire::Styles::select(const IsASelector& selector,
    QWidget& source) {
  if(selector.is_instance(source)) {
    return std::vector{&source};
  }
  return {};
}
