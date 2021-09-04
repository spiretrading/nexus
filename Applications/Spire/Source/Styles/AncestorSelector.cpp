#include "Spire/Styles/AncestorSelector.hpp"
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
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
