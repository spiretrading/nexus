#include "Spire/Styles/Property.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::type_index Property::get_type() const {
  return m_property.type();
}

bool Property::operator ==(const Property& property) const {
  return m_is_equal(*this, property);
}

bool Property::operator !=(const Property& property) const {
  return !(*this == property);
}
