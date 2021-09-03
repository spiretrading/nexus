#include "Spire/Styles/ChildSelector.hpp"
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

ChildSelector::ChildSelector(Selector base, Selector child)
  : m_base(std::move(base)),
    m_child(std::move(child)) {}

const Selector& ChildSelector::get_base() const {
  return m_base;
}

const Selector& ChildSelector::get_child() const {
  return m_child;
}

bool ChildSelector::operator ==(const ChildSelector& selector) const {
  return m_base == selector.get_base() && m_child == selector.get_child();
}

bool ChildSelector::operator !=(const ChildSelector& selector) const {
  return !(*this == selector);
}

ChildSelector Spire::Styles::operator >(Selector base, Selector child) {
  return ChildSelector(std::move(base), std::move(child));
}

SelectConnection Spire::Styles::select(const ChildSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_child(),
    [] (const Stylist& stylist) {
      auto children = std::unordered_set<const Stylist*>();
      for(auto child : stylist.get_widget().children()) {
        if(child->isWidgetType()) {
          children.insert(&find_stylist(static_cast<QWidget&>(*child)));
        }
      }
      return children;
    }), base, on_update);
}
