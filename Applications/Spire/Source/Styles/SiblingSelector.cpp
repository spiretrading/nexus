#include "Spire/Styles/SiblingSelector.hpp"
#include <unordered_set>
#include <QLayout>
#include <QWidget>

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

bool SiblingSelector::is_match(const SiblingSelector& selector) const {
  return m_base.is_match(selector.get_base()) &&
    m_sibling.is_match(selector.get_sibling());
}

SiblingSelector Spire::Styles::operator %(Selector base, Selector sibling) {
  return SiblingSelector(std::move(base), std::move(sibling));
}

std::vector<QWidget*> Spire::Styles::select(
    const SiblingSelector& selector, QWidget& source) {
  if(!source.parentWidget()) {
    return {};
  }
  auto bases = select(selector.get_base(), source);
  auto selection = std::unordered_set<QWidget*>();
  for(auto base : bases) {
    auto siblings = source.parent()->children();
    auto i = 0;
    while(i != siblings.size()) {
      auto child = siblings[i];
      if(child != base) {
        if(auto c = qobject_cast<QWidget*>(child)) {
          auto child_selection = select(selector.get_sibling(), *c);
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
