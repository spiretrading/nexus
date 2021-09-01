#include "Spire/Styles/SiblingSelector.hpp"
#include <unordered_set>
#include <QLayout>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
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

SelectConnection Spire::Styles::select(const SiblingSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_sibling(),
    [] (const Stylist& stylist) {
      auto siblings = std::unordered_set<const Stylist*>();
      auto widgets = stylist.get_widget().parent()->children();
      auto i = 0;
      while(i != widgets.size()) {
        auto child = widgets[i];
        if(child != &stylist.get_widget()) {
          if(auto sibling = qobject_cast<QWidget*>(child)) {
            siblings.insert(&find_stylist(*sibling));
          } else if(auto layout = qobject_cast<QLayout*>(child)) {
            widgets.append(layout->children());
          }
        }
        ++i;
      }
      return siblings;
    }), base, on_update);
}
