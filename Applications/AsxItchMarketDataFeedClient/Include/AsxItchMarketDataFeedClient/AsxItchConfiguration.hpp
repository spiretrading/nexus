#ifndef NEXUS_ASX_ITCH_CONFIGURATION_HPP
#define NEXUS_ASX_ITCH_CONFIGURATION_HPP
#include <string>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus::MarketDataService {

  /** Stores the configuration of an ASX ITCH parser. */
  struct AsxItchConfiguration {

    /** Whether to log messages. */
    bool m_isLoggingMessages;

    /** The Glimpse username. */
    std::string m_glimpseUsername;

    /** The Glimpse password. */
    std::string m_glimpsePassword;

    /** Whether trades should be treated as a time and sale. */
    bool m_isTimeAndSaleFeed;

    /** The Market disseminating the data. */
    MarketDatabase::Entry m_market;

    /** The default MPID to attribute Orders to. */
    std::string m_defaultMpid;

    /** Whether to consolidate all Orders as originating from a single MPID. */
    bool m_consolidateMpids;
  };
}

#endif
