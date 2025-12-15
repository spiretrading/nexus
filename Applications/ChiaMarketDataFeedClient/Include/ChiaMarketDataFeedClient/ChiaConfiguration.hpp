#ifndef NEXUS_CHIA_CONFIGURATION_HPP
#define NEXUS_CHIA_CONFIGURATION_HPP
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores the configuration for a CHIA market data parser. */
  struct ChiaConfiguration {

    /** Whether to log messages. */
    bool m_is_logging_messages;

    /** The market's CountryCode. */
    CountryCode m_country;

    /** The market on which the Security is listed. */
    Venue m_primary_venue;

    /** The Market disseminating the data. */
    Venue m_disseminating_venue;

    /** The MPID to display. */
    std::string m_mpid;

    /** Whether trades should be treated as a time and sale. */
    bool m_is_time_and_sale_feed;
  };
}

#endif
