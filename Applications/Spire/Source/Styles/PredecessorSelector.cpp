#include "Spire/Styles/PredecessorSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

PredecessorSelector::PredecessorSelector(Selector base, Selector predecessor)
  : m_base(std::move(base)),
    m_predecessor(std::move(predecessor)) {}

const Selector& PredecessorSelector::get_base() const {
  return m_base;
}

const Selector& PredecessorSelector::get_predecessor() const {
  return m_predecessor;
}

bool PredecessorSelector::operator ==(
    const PredecessorSelector& selector) const {
  return m_base == selector.get_base() &&
    m_predecessor == selector.get_predecessor();
}

bool PredecessorSelector::operator !=(
    const PredecessorSelector& selector) const {
  return !(*this == selector);
}

SelectConnection Spire::Styles::select(const PredecessorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return {};
}
