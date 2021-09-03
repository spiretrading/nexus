#include "Spire/Styles/ParentSelector.hpp"
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
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

SelectConnection Spire::Styles::select(const ParentSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(),
    selector.get_parent(), [] (const Stylist& stylist) {
      if(auto parent = stylist.get_widget().parentWidget()) {
        return std::unordered_set<const Stylist*>{&find_stylist(*parent)};
      }
      return std::unordered_set<const Stylist*>();
    }), base, on_update);
}
