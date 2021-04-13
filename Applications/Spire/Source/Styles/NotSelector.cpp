#include "Spire/Styles/NotSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

NotSelector::NotSelector(Selector selector)
  : m_selector(std::move(selector)) {}

const Selector& NotSelector::get_selector() const {
  return m_selector;
}

bool NotSelector::operator ==(const NotSelector& selector) const {
  return m_selector == selector.get_selector();
}

bool NotSelector::operator !=(const NotSelector& selector) const {
  return !(*this == selector);
}

NotSelector Spire::Styles::operator !(Selector selector) {
  return NotSelector(std::move(selector));
}

std::vector<Stylist*> Spire::Styles::select(
    const NotSelector& selector, Stylist& source) {
  auto selection = select(selector.get_selector(), source);
  if(selection.empty()) {
    return std::vector{&source};
  }
  return {};
}
