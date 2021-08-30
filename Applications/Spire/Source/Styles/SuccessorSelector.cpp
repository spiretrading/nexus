#include "Spire/Styles/SuccessorSelector.hpp"
#include <QWidget>
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

SuccessorSelector::SuccessorSelector(Selector base, Selector successor)
  : m_base(std::move(base)),
    m_successor(std::move(successor)) {}

const Selector& SuccessorSelector::get_base() const {
  return m_base;
}

const Selector& SuccessorSelector::get_successor() const {
  return m_successor;
}

bool SuccessorSelector::operator ==(const SuccessorSelector& selector) const {
  return m_base == selector.get_base() &&
    m_successor == selector.get_successor();
}

bool SuccessorSelector::operator !=(const SuccessorSelector& selector) const {
  return !(*this == selector);
}

SuccessorSelector Spire::Styles::operator >(Selector base, Selector successor) {
  return SuccessorSelector(std::move(base), std::move(successor));
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const SuccessorSelector& selector, std::unordered_set<Stylist*> sources) {
  auto selection = std::unordered_set<Stylist*>();
  auto is_flipped = selector.get_base().get_type() == typeid(FlipSelector);
  for(auto source : select(selector.get_base(), std::move(sources))) {
    for(auto child : source->get_widget().children()) {
      if(child->isWidgetType()) {
        auto child_selection = select(selector.get_successor(),
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
    const SuccessorSelector& selector, QWidget& source) {
  auto reach = std::unordered_set<QWidget*>();
  for(auto base : build_reach(selector.get_base(), source)) {
    reach.insert(base);
    for(auto child : base->children()) {
      if(child->isWidgetType()) {
        auto child_reach =
          build_reach(selector.get_successor(), *static_cast<QWidget*>(child));
        reach.insert(child_reach.begin(), child_reach.end());
      }
    }
  }
  return reach;
}
