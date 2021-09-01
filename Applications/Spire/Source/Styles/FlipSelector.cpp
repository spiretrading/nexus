#include "Spire/Styles/FlipSelector.hpp"
#include <QWidget>
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

FlipSelector::FlipSelector(Selector selector)
  : m_selector(std::move(selector)) {}

const Selector& FlipSelector::get_selector() const {
  return m_selector;
}

bool FlipSelector::operator ==(const FlipSelector& selector) const {
  return m_selector == selector.get_selector();
}

bool FlipSelector::operator !=(const FlipSelector& selector) const {
  return !(*this == selector);
}

FlipSelector Spire::Styles::operator +(Selector selector) {
  return FlipSelector(std::move(selector));
}

SelectConnection Spire::Styles::select(const FlipSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(selector.get_selector(), base, on_update);
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const FlipSelector& selector, std::unordered_set<Stylist*> sources) {
  return select(selector.get_selector(), std::move(sources));
}

std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const FlipSelector& selector, QWidget& source) {
  return build_reach(selector.get_selector(), source);
}
