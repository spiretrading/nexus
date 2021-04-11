#include "Spire/Styles/DisambiguateSelector.hpp"
#include <QWidget>
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

DisambiguateSelector::DisambiguateSelector(Selector selector)
  : m_selector(std::move(selector)) {}

const Selector& DisambiguateSelector::get_selector() const {
  return m_selector;
}

bool DisambiguateSelector::operator ==(
    const DisambiguateSelector& selector) const {
  return m_selector == selector.get_selector();
}

bool DisambiguateSelector::operator !=(
    const DisambiguateSelector& selector) const {
  return !(*this == selector);
}

DisambiguateSelector Spire::Styles::operator +(Selector selector) {
  return DisambiguateSelector(std::move(selector));
}

std::vector<Stylist*> Spire::Styles::select(
    const DisambiguateSelector& selector, Stylist& source) {
  return select(selector.get_selector(), source);
}

std::vector<QWidget*> Spire::Styles::build_reach(
    const DisambiguateSelector& selector, QWidget& source) {
  return build_reach(selector.get_selector(), source);
}
