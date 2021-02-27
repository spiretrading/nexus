#include "Spire/Styles/Selector.hpp"

using namespace Spire;
using namespace Spire::Styles;

Selector::Selector(Any any)
  : m_selector(std::move(any)) {}

std::type_index Selector::get_type() const {
  return m_selector.type();
}
