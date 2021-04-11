#include "Spire/Styles/ParentSelector.hpp"
#include <unordered_set>
#include <QWidget>
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

ParentSelector::ParentSelector(Selector base, Selector parent)
  : m_base(std::move(base)),
    m_parent(std::move(parent)) {}

const Selector& ParentSelector::get_base() const {
  return m_base;
}

const Selector& ParentSelector::get_parent() const {
  return m_parent;
}

bool ParentSelector::operator ==(const ParentSelector& selector) const {
  return m_base == selector.get_base() && m_parent == selector.get_parent();
}

bool ParentSelector::operator !=(const ParentSelector& selector) const {
  return !(*this == selector);
}

std::vector<Stylist*> Spire::Styles::select(
    const ParentSelector& selector, Stylist& source) {
  auto selection = std::unordered_set<Stylist*>();
  auto bases = select(selector.get_base(), source);
  for(auto base : bases) {
    if(auto parent = base->get_widget().parentWidget()) {
      auto parent_selection = select(selector.get_parent(),
        find_stylist(*parent));
      selection.insert(parent_selection.begin(), parent_selection.end());
    }
  }
  return std::vector(selection.begin(), selection.end());
}

std::vector<QWidget*> Spire::Styles::build_reach(
    const ParentSelector& selector, QWidget& source) {
  auto reach = std::unordered_set<QWidget*>();
  auto bases = build_reach(selector.get_base(), source);
  reach.insert(bases.begin(), bases.end());
  for(auto base : bases) {
    if(auto parent = base->parentWidget()) {
      auto parent_reach = build_reach(selector.get_parent(), *parent);
      reach.insert(parent_reach.begin(), parent_reach.end());
    }
  }
  return std::vector(reach.begin(), reach.end());
}
