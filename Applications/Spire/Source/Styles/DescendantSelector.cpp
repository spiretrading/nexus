#include "Spire/Styles/DescendantSelector.hpp"
#include <deque>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/FlipSelector.hpp"
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

std::unordered_set<Stylist*> Spire::Styles::select(
    const DescendantSelector& selector, std::unordered_set<Stylist*> sources) {
  auto selection = std::unordered_set<Stylist*>();
  auto is_flipped = selector.get_base().get_type() == typeid(FlipSelector);
  for(auto source : select(selector.get_base(), std::move(sources))) {
    auto descendants = std::deque<QWidget*>();
    for(auto child : source->get_widget().children()) {
      if(child->isWidgetType()) {
        descendants.push_back(static_cast<QWidget*>(child));
      }
    }
    while(!descendants.empty()) {
      auto descendant = descendants.front();
      descendants.pop_front();
      auto descendant_selection = select(selector.get_descendant(),
        find_stylist(*static_cast<QWidget*>(descendant)));
      if(!descendant_selection.empty()) {
        if(is_flipped) {
          selection.insert(source);
          break;
        } else {
          selection.insert(
            descendant_selection.begin(), descendant_selection.end());
        }
      }
      for(auto child : descendant->children()) {
        if(child->isWidgetType()) {
          descendants.push_back(static_cast<QWidget*>(child));
        }
      }
    }
  }
  return selection;
}

std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const DescendantSelector& selector, QWidget& source) {
  auto reach = build_reach(selector.get_base(), source);
  auto descendants = std::deque<QWidget*>();
  descendants.insert(descendants.end(), reach.begin(), reach.end());
  while(!descendants.empty()) {
    auto base = descendants.front();
    descendants.pop_front();
    for(auto child : base->children()) {
      if(child->isWidgetType()) {
        auto widget = static_cast<QWidget*>(child);
        descendants.push_back(widget);
        auto child_reach = build_reach(selector.get_descendant(), *widget);
        reach.insert(child_reach.begin(), child_reach.end());
      }
    }
  }
  return reach;
}
