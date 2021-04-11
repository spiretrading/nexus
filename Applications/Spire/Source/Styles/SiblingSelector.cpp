#include "Spire/Styles/SiblingSelector.hpp"
#include <unordered_set>
#include <QLayout>
#include <QWidget>
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

SiblingSelector::SiblingSelector(Selector base, Selector sibling)
  : m_base(std::move(base)),
    m_sibling(std::move(sibling)) {}

const Selector& SiblingSelector::get_base() const {
  return m_base;
}

const Selector& SiblingSelector::get_sibling() const {
  return m_sibling;
}

bool SiblingSelector::operator ==(const SiblingSelector& selector) const {
  return m_base == selector.get_base() && m_sibling == selector.get_sibling();
}

bool SiblingSelector::operator !=(const SiblingSelector& selector) const {
  return !(*this == selector);
}

SiblingSelector Spire::Styles::operator %(Selector base, Selector sibling) {
  return SiblingSelector(std::move(base), std::move(sibling));
}

std::vector<Stylist*> Spire::Styles::select(
    const SiblingSelector& selector, Stylist& source) {
  if(!source.get_widget().parentWidget()) {
    return {};
  }
  auto bases = select(selector.get_base(), source);
  auto selection = std::unordered_set<Stylist*>();
  for(auto base : bases) {
    auto siblings = source.get_widget().parent()->children();
    auto i = 0;
    while(i != siblings.size()) {
      auto child = siblings[i];
      if(child != &base->get_widget()) {
        if(auto c = qobject_cast<QWidget*>(child)) {
          auto child_selection = select(selector.get_sibling(),
            find_stylist(*c));
          selection.insert(child_selection.begin(), child_selection.end());
        } else if(auto layout = qobject_cast<QLayout*>(child)) {
          siblings.append(layout->children());
        }
      }
      ++i;
    }
  }
  return std::vector(selection.begin(), selection.end());
}
