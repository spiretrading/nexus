#ifndef SPIRE_STYLES_ENUM_PROPERTY_HPP
#define SPIRE_STYLES_ENUM_PROPERTY_HPP
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Stores a style property for enum types.
   * @param <E> The type of enum the property evaluates to.
   */
  template<typename E>
  using EnumProperty = BasicProperty<E, E>;
}

#endif
