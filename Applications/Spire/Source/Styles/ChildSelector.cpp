#include "Spire/Styles/ChildSelector.hpp"
#include <set>
#include <QWidget>

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

bool ChildSelector::is_match(const ChildSelector& selector) const {
  return m_base.is_match(selector.get_base()) &&
    m_child.is_match(selector.get_child());
}

ChildSelector Spire::Styles::operator >(Selector base, Selector child) {
  return ChildSelector(std::move(base), std::move(child));
}

std::vector<QWidget*> Spire::Styles::select(const ChildSelector& selector,
    QWidget& source) {
  auto selection = std::set<QWidget*>();
  auto bases = select(selector.get_base(), source);
  for(auto base : bases) {
    for(auto& child : base->children()) {
      if(child->isWidgetType()) {
        auto child_selection = select(selector.get_child(),
          *static_cast<QWidget*>(child));
        selection.insert(child_selection.begin(), child_selection.end());
      }
    }
  }
  return std::vector(selection.begin(), selection.end());
}
