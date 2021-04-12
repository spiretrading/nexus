#include "Spire/Styles/DescendantSelector.hpp"
#include <deque>
#include <unordered_set>
#include <QWidget>
#include "Spire/Styles/DisambiguateSelector.hpp"
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

std::vector<Stylist*> Spire::Styles::select(const DescendantSelector& selector,
    Stylist& source) {
  auto selection = std::unordered_set<Stylist*>();
  auto bases = select(selector.get_base(), source);
  auto is_disambiguated = selector.get_base().get_type() ==
    typeid(DisambiguateSelector);
  for(auto base : bases) {
    auto descendants = std::deque<QWidget*>();
    for(auto child : base->get_widget().children()) {
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
        if(is_disambiguated) {
          selection.insert(base);
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
  return std::vector(selection.begin(), selection.end());
}
