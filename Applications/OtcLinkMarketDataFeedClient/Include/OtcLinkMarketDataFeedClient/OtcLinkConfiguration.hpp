#ifndef OTC_LINK_CONFIGURATION_HPP
#define OTC_LINK_CONFIGURATION_HPP
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores configuration for parsing OTC Link messages. */
  struct OtcLinkConfiguration {

    /** Whether to log messages to standard output. */
    bool m_is_logging_messages;

    /** The Venue disseminating messages. */
    Venue m_venue;
  };
}

#endif
