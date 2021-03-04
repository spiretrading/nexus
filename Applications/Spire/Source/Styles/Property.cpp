#include "Spire/Styles/Property.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::type_index Property::get_type() const {
  return m_property.type();
}
