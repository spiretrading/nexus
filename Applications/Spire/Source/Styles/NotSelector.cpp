#include "Spire/Styles/NotSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

NotSelector::NotSelector(Selector selector)
  : m_selector(std::move(selector)) {}

const Selector& NotSelector::get_selector() const {
  return m_selector;
}

bool NotSelector::is_match(const NotSelector& selector) const {
  return m_selector.is_match(selector.get_selector());
}

NotSelector Spire::Styles::operator !(Selector selector) {
  return NotSelector(std::move(selector));
}

std::vector<QWidget*> Spire::Styles::select(
    const NotSelector& selector, QWidget& source) {
  auto selection = select(selector.get_selector(), source);
  if(selection.empty()) {
    return std::vector{&source};
  }
  return {};
}
