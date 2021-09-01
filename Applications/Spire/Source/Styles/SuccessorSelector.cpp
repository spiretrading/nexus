#include "Spire/Styles/SuccessorSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

SuccessorSelector::SuccessorSelector(Selector base, Selector successor)
  : m_base(std::move(base)),
    m_successor(std::move(successor)) {}

const Selector& SuccessorSelector::get_base() const {
  return m_base;
}

const Selector& SuccessorSelector::get_successor() const {
  return m_successor;
}

bool SuccessorSelector::operator ==(const SuccessorSelector& selector) const {
  return m_base == selector.get_base() &&
    m_successor == selector.get_successor();
}

bool SuccessorSelector::operator !=(const SuccessorSelector& selector) const {
  return !(*this == selector);
}

SuccessorSelector Spire::Styles::operator >(Selector base, Selector successor) {
  return SuccessorSelector(std::move(base), std::move(successor));
}

SelectConnection Spire::Styles::select(const SuccessorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return {};
}
