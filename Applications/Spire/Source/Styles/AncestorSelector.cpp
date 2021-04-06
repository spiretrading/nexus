#include "Spire/Styles/AncestorSelector.hpp"
#include <set>
#include "Spire/Styles/StyledWidget.hpp"

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

bool AncestorSelector::is_match(const AncestorSelector& selector) const {
  return m_base.is_match(selector.get_base()) &&
    m_ancestor.is_match(selector.get_ancestor());
}

AncestorSelector Spire::Styles::operator <<(Selector base, Selector ancestor) {
  return AncestorSelector(std::move(base), std::move(ancestor));
}

std::vector<QWidget*> Spire::Styles::select(
    const AncestorSelector& selector, QWidget& source) {
  auto selection = std::set<QWidget*>();
  auto bases = select(selector.get_base(), source);
  for(auto base : bases) {
    auto ancestor = base->parentWidget();
    while(ancestor) {
      auto ancestor_selection = select(selector.get_ancestor(), *ancestor);
      selection.insert(ancestor_selection.begin(), ancestor_selection.end());
      ancestor = ancestor->parentWidget();
    }
  }
  return std::vector(selection.begin(), selection.end());
}
