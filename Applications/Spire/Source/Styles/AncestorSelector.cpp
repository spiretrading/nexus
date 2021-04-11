#include "Spire/Styles/AncestorSelector.hpp"
#include <unordered_set>
#include <QWidget>
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

std::vector<Stylist*> Spire::Styles::select(
    const AncestorSelector& selector, Stylist& source) {
  auto selection = std::unordered_set<Stylist*>();
  auto bases = select(selector.get_base(), source);
  for(auto base : bases) {
    auto ancestor = base->get_widget().parentWidget();
    while(ancestor) {
      auto ancestor_selection = select(selector.get_ancestor(),
        find_stylist(*ancestor));
      selection.insert(ancestor_selection.begin(), ancestor_selection.end());
      ancestor = ancestor->parentWidget();
    }
  }
  return std::vector(selection.begin(), selection.end());
}
