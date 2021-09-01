#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/StyleSheet.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::type_index Selector::get_type() const {
  return m_selector.type();
}

bool Selector::operator ==(const Selector& selector) const {
  return m_is_equal(*this, selector);
}

bool Selector::operator !=(const Selector& selector) const {
  return !(*this == selector);
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const Selector& selector, std::unordered_set<Stylist*> source) {
  return selector.m_select(selector, std::move(source));
}

std::unordered_set<Stylist*>
    Spire::Styles::select(const Selector& selector, Stylist& source) {
  return select(selector, std::unordered_set{&source});
}

SelectConnection Spire::Styles::select(const Selector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return selector.m_select_connection(selector, base, on_update);
}
 
std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const Selector& selector, QWidget& source) {
  return selector.m_reach(selector, source);
}

std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const StyleSheet& style, QWidget& source) {
  auto reach = std::unordered_set<QWidget*>();
  for(auto& rule : style.get_rules()) {
    auto sub_reach = build_reach(rule.get_selector(), source);
    reach.insert(sub_reach.begin(), sub_reach.end());
  }
  return reach;
}
