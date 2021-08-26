#include "Spire/Styles/ChildSelector.hpp"
#include <QWidget>
#include "Spire/Styles/FlipSelector.hpp"
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

std::unordered_set<Stylist*> Spire::Styles::select(
    const ChildSelector& selector, std::unordered_set<Stylist*> sources) {
  auto selection = std::unordered_set<Stylist*>();
  auto is_flipped = selector.get_base().get_type() == typeid(FlipSelector);
  for(auto source : select(selector.get_base(), std::move(sources))) {
    for(auto child : source->get_widget().children()) {
      if(child->isWidgetType()) {
        auto child_selection = select(selector.get_child(),
          find_stylist(*static_cast<QWidget*>(child)));
        if(!child_selection.empty()) {
          if(is_flipped) {
            selection.insert(source);
            break;
          } else {
            selection.insert(child_selection.begin(), child_selection.end());
          }
        }
      }
    }
  }
  return selection;
}

std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const ChildSelector& selector, QWidget& source) {
  auto reach = std::unordered_set<QWidget*>();
  for(auto base : build_reach(selector.get_base(), source)) {
    reach.insert(base);
    for(auto child : base->children()) {
      if(child->isWidgetType()) {
        auto child_reach =
          build_reach(selector.get_child(), *static_cast<QWidget*>(child));
        reach.insert(child_reach.begin(), child_reach.end());
      }
    }
  }
  return reach;
}
