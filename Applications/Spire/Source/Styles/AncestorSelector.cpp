#include "Spire/Styles/AncestorSelector.hpp"
#include <deque>
#include <unordered_map>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

AncestorSelector::AncestorSelector(Selector base, Selector ancestor)
  : m_base(std::move(base)),
    m_ancestor(std::move(ancestor)) {}

const Selector& AncestorSelector::get_base() const {
  return m_base;
}

const Selector& AncestorSelector::get_ancestor() const {
  return m_ancestor;
}

bool AncestorSelector::operator ==(const AncestorSelector& selector) const {
  return m_base == selector.get_base() && m_ancestor == selector.get_ancestor();
}

bool AncestorSelector::operator !=(const AncestorSelector& selector) const {
  return !(*this == selector);
}

AncestorSelector Spire::Styles::operator <<(Selector base, Selector ancestor) {
  return AncestorSelector(std::move(base), std::move(ancestor));
}

SelectConnection Spire::Styles::select(const AncestorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_ancestor(),
    [] (const Stylist& stylist) {
      auto ancestors = std::unordered_set<const Stylist*>();
      auto ancestor = stylist.get_widget().parentWidget();
      while(ancestor) {
        auto& stylist = find_stylist(*ancestor);
        ancestors.insert(&find_stylist(*ancestor));
        ancestor = ancestor->parentWidget();
      }
      return ancestors;
    }), base, on_update);
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const AncestorSelector& selector, std::unordered_set<Stylist*> sources) {
  auto is_flipped = selector.get_base().get_type() == typeid(FlipSelector);
  auto selection = std::unordered_set<Stylist*>();
  for(auto source : select(selector.get_base(), std::move(sources))) {
    auto ancestor = source->get_widget().parentWidget();
    while(ancestor) {
      auto ancestor_selection =
        select(selector.get_ancestor(), find_stylist(*ancestor));
      if(!ancestor_selection.empty()) {
        if(is_flipped) {
          selection.insert(source);
          break;
        } else {
          selection.insert(
            ancestor_selection.begin(), ancestor_selection.end());
        }
      }
      ancestor = ancestor->parentWidget();
    }
  }
  return selection;
}

std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const AncestorSelector& selector, QWidget& source) {
  auto reach = build_reach(selector.get_base(), source);
  auto ancestors = std::deque<QWidget*>();
  for(auto base : reach) {
    if(auto parent = base->parentWidget()) {
      ancestors.push_back(parent);
    }
  }
  while(!ancestors.empty()) {
    auto ancestor = ancestors.front();
    ancestors.pop_front();
    auto ancestor_reach = build_reach(selector.get_ancestor(), *ancestor);
    reach.insert(ancestor_reach.begin(), ancestor_reach.end());
    if(auto parent = ancestor->parentWidget()) {
      ancestors.push_back(parent);
    }
  }
  return reach;
}
