#ifndef SPIRE_ADDITIONAL_TAG_HPP
#define SPIRE_ADDITIONAL_TAG_HPP
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Tag.hpp"
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /** Stores a tag and its meta-data. */
  struct AdditionalTag {

    /** The schema storing this tag's meta-data. */
    std::shared_ptr<AdditionalTagSchema> m_schema;

    /** The tag's value. */
    boost::optional<Nexus::Tag::Type> m_value;

  };
}

#endif
