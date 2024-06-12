#ifndef SPIRE_ADDITIONAL_TAG_HPP
#define SPIRE_ADDITIONAL_TAG_HPP
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Tag.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /** Stores an additional tag used to submit an Order. */
  struct AdditionalTag {

    /** The tag's key. */
    int m_key;

    /** The tag's value. */
    boost::optional<Nexus::Tag::Type> m_value;
  };
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Spire::AdditionalTag> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, Spire::AdditionalTag& value, unsigned int version) {
      shuttle.Shuttle("key", value.m_key);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}

#endif
