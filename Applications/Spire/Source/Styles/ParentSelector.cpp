#include "Spire/Styles/ParentSelector.hpp"
#include <QWidget>
#include "Spire/Styles/FlipSelector.hpp"
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

std::unordered_set<Stylist*> Spire::Styles::select(
    const ParentSelector& selector, std::unordered_set<Stylist*> sources) {
  auto is_flipped = selector.get_base().get_type() == typeid(FlipSelector);
  auto selection = std::unordered_set<Stylist*>();
  for(auto source : select(selector.get_base(), std::move(sources))) {
    if(auto parent = source->get_widget().parentWidget()) {
      auto parent_selection = select(selector.get_parent(),
        find_stylist(*parent));
      if(!parent_selection.empty()) {
        if(is_flipped) {
          selection.insert(source);
        } else {
          selection.insert(parent_selection.begin(), parent_selection.end());
        }
      }
    }
  }
  return selection;
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
