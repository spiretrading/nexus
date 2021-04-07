#include "Spire/Styles/DescendantSelector.hpp"
#include <deque>
#include <unordered_set>
#include <QWidget>

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

bool DescendantSelector::is_match(const DescendantSelector& selector) const {
  return m_base.is_match(selector.get_base()) &&
    m_descendant.is_match(selector.get_descendant());
}

DescendantSelector Spire::Styles::operator >>(
    Selector base, Selector descendant) {
  return DescendantSelector(std::move(base), std::move(descendant));
}

std::vector<QWidget*> Spire::Styles::select(const DescendantSelector& selector,
    QWidget& source) {
  auto selection = std::unordered_set<QWidget*>();
  auto bases = select(selector.get_base(), source);
  for(auto base : bases) {
    auto descendants = std::deque<QWidget*>();
    for(auto child : base->children()) {
      if(child->isWidgetType()) {
        descendants.push_back(static_cast<QWidget*>(child));
      }
    }
    while(!descendants.empty()) {
      auto descendant = descendants.front();
      descendants.pop_front();
      auto descendant_selection = select(selector.get_descendant(),
        *static_cast<QWidget*>(descendant));
      selection.insert(
        descendant_selection.begin(), descendant_selection.end());
      for(auto child : descendant->children()) {
        if(child->isWidgetType()) {
          descendants.push_back(static_cast<QWidget*>(child));
        }
      }
    }
  }
  return std::vector(selection.begin(), selection.end());
}
