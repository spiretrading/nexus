#ifndef NEXUS_ASX_ITCH_CONFIGURATION_HPP
#define NEXUS_ASX_ITCH_CONFIGURATION_HPP
#include <string>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores the configuration of an ASX ITCH parser. */
  struct AsxItchConfiguration {

    /** Whether to log messages. */
    bool m_is_logging_messages;

    /** The Glimpse username. */
    std::string m_glimpse_username;

    /** The Glimpse password. */
    std::string m_glimpse_password;

    /** Whether trades should be treated as a time and sale. */
    bool m_is_time_and_sale_feed;

    /** The venue disseminating the data. */
    VenueDatabase::Entry m_venue;

    /** The default MPID to attribute Orders to. */
    std::string m_default_mpid;

    /** Whether to consolidate all Orders as originating from a single MPID. */
    bool m_consolidate_mpids;
  };
}

#endif
