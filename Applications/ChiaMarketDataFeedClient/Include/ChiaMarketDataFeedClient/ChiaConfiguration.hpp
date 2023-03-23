#ifndef NEXUS_CHIA_CONFIGURATION_HPP
#define NEXUS_CHIA_CONFIGURATION_HPP
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus::MarketDataService {

  /** Stores the configuration for a CHIA market data parser. */
  struct ChiaConfiguration {

    /** Whether to log messages. */
    bool m_isLoggingMessages;

    /** The market's CountryCode. */
    CountryCode m_country;

    /** The market on which the Security is listed. */
    MarketCode m_primaryMarket;

    /** The Market disseminating the data. */
    MarketCode m_disseminatingMarket;

    /** The MPID to display. */
    std::string m_mpid;

    /** Whether trades should be treated as a time and sale. */
    bool m_isTimeAndSaleFeed;
  };
}

#endif
