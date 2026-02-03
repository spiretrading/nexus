#ifndef OTC_LINK_CONFIGURATION_HPP
#define OTC_LINK_CONFIGURATION_HPP
#include "Nexus/Definitions/Venue.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkChannelId.hpp"

namespace Nexus {

  /** Stores configuration for parsing OTC Link messages. */
  struct OtcLinkConfiguration {

    /** Whether to log messages to standard output. */
    bool m_is_logging_messages;

    /** The channel ID used for recovery requests. */
    OtcLinkChannelId m_recovery_channel;

    /** The Venue disseminating messages. */
    Venue m_venue;
  };
}

#endif
