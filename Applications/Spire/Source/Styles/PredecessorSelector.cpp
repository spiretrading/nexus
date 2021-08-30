#include "Spire/Styles/PredecessorSelector.hpp"
#include <QWidget>
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

PredecessorSelector::PredecessorSelector(Selector base, Selector predecessor)
  : m_base(std::move(base)),
    m_predecessor(std::move(predecessor)) {}

const Selector& PredecessorSelector::get_base() const {
  return m_base;
}

const Selector& PredecessorSelector::get_predecessor() const {
  return m_predecessor;
}

bool PredecessorSelector::operator ==(
    const PredecessorSelector& selector) const {
  return m_base == selector.get_base() &&
    m_predecessor == selector.get_predecessor();
}

bool PredecessorSelector::operator !=(
    const PredecessorSelector& selector) const {
  return !(*this == selector);
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const PredecessorSelector& selector, std::unordered_set<Stylist*> sources) {
  auto is_flipped = selector.get_base().get_type() == typeid(FlipSelector);
  auto selection = std::unordered_set<Stylist*>();
  for(auto source : select(selector.get_base(), std::move(sources))) {
    auto predecessor = source->get_widget().parentWidget();
    while(predecessor) {
      auto predecessor_selection =
        select(selector.get_predecessor(), find_stylist(*predecessor));
      if(!predecessor_selection.empty()) {
        if(is_flipped) {
          selection.insert(source);
        } else {
          selection.insert(
            predecessor_selection.begin(), predecessor_selection.end());
        }
        break;
      }
      predecessor = predecessor->parentWidget();
    }
  }
  return selection;
}

std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const PredecessorSelector& selector, QWidget& source) {
  auto reach = std::unordered_set<QWidget*>();
  for(auto base : build_reach(selector.get_base(), source)) {
    reach.insert(base);
    if(auto parent = base->parentWidget()) {
      auto parent_reach = build_reach(selector.get_predecessor(), *parent);
      reach.insert(parent_reach.begin(), parent_reach.end());
    }
  }
  return reach;
}
