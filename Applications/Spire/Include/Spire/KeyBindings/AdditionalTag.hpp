#ifndef SPIRE_ADDITIONAL_TAG_HPP
#define SPIRE_ADDITIONAL_TAG_HPP
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Tag.hpp"

namespace Spire {

  /** Stores an additional tag used to submit an Order. */
  struct AdditionalTag {

    /** The tag's key. */
    int m_key;

    /** The tag's value. */
    boost::optional<Nexus::Tag::Type> m_value;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Spire::AdditionalTag> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Spire::AdditionalTag& value, unsigned int version) const {
      shuttle.shuttle("key", value.m_key);
      shuttle.shuttle("value", value.m_value);
    }
  };
}

#endif
