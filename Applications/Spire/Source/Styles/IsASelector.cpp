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

std::unordered_set<Stylist*> Spire::Styles::select(
    const IsASelector& selector, std::unordered_set<Stylist*> sources) {
  for(auto i = sources.begin(); i != sources.end();) {
    auto& source = **i;
    if(source.get_pseudo_element() ||
        !selector.is_instance(source.get_widget())) {
      i = sources.erase(i);
    } else {
      ++i;
    }
  }
  return sources;
}

std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const IsASelector& selector, QWidget& source) {
  if(selector.is_instance(source)) {
    return {&source};
  }
  return {};
}
