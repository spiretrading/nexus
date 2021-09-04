#include "Spire/Styles/DescendantSelector.hpp"
#include <deque>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

DescendantSelector::DescendantSelector(Selector base, Selector descendant)
  : m_base(std::move(base)),
    m_descendant(std::move(descendant)) {}

const Selector& DescendantSelector::get_base() const {
  return m_base;
}

const Selector& DescendantSelector::get_descendant() const {
  return m_descendant;
}

bool DescendantSelector::operator ==(const DescendantSelector& selector) const {
  return m_base == selector.get_base() &&
    m_descendant == selector.get_descendant();
}

bool DescendantSelector::operator !=(const DescendantSelector& selector) const {
  return !(*this == selector);
}

DescendantSelector Spire::Styles::operator >>(
    Selector base, Selector descendant) {
  return DescendantSelector(std::move(base), std::move(descendant));
}

SelectConnection Spire::Styles::select(const DescendantSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(),
    selector.get_descendant(), [] (const Stylist& stylist) {
      auto descendants = std::unordered_set<const Stylist*>();
      auto breadth_traversal = std::deque<const QWidget*>();
      breadth_traversal.push_back(&stylist.get_widget());
      while(!breadth_traversal.empty()) {
        auto widget = breadth_traversal.front();
        breadth_traversal.pop_front();
        for(auto child : widget->children()) {
          if(child->isWidgetType()) {
            auto& child_widget = static_cast<QWidget&>(*child);
            breadth_traversal.push_back(&child_widget);
            descendants.insert(&find_stylist(child_widget));
          }
        }
      }
      return descendants;
    }), base, on_update);
}
