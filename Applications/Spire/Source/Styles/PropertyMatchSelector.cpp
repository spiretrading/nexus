#include "Spire/Styles/PropertyMatchSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

PropertyMatchSelector::PropertyMatchSelector(Property property)
  : m_property(std::move(property)) {}

const Property& PropertyMatchSelector::get_property() const {
  return m_property;
}

PropertyMatchSelector Spire::Styles::matches(Property property) {
  return PropertyMatchSelector(std::move(property));
}

SelectConnection Spire::Styles::select(const PropertyMatchSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  struct Executor {
    Property m_property;
    const Stylist* m_base;
    SelectionUpdateSignal m_on_update;
    bool m_is_selected;
    scoped_connection m_style_connection;

    Executor(const PropertyMatchSelector& selector, const Stylist& base,
      const SelectionUpdateSignal& on_update)
      : m_property(selector.get_property()),
        m_base(&base),
        m_on_update(on_update),
        m_is_selected(false),
        m_style_connection(connect_style_signal(
          base.get_widget(), std::bind_front(&Executor::on_style, this))) {}

    void on_style() {
      auto& block = m_base->get_computed_block();
      auto is_missing_property = true;
      for(auto& property : block) {
        if(property == m_property) {
          is_missing_property = false;
          break;
        }
      }
      if(is_missing_property) {
        if(m_is_selected) {
          m_is_selected = false;
          m_on_update({}, {m_base});
        }
      } else if(!m_is_selected) {
        m_is_selected = true;
        m_on_update({m_base}, {});
      }
    }
  };
  return SelectConnection(
    std::make_unique<Executor>(selector, base, on_update));
}

std::size_t std::hash<PropertyMatchSelector>::operator ()(
    const PropertyMatchSelector& selector) const {
  return std::hash<Property>()(selector.get_property());
}
