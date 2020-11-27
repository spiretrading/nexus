#ifndef NEXUS_TMX_TL1_CONFIGURATION_HPP
#define NEXUS_TMX_TL1_CONFIGURATION_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus::MarketDataService {

  /** Stores the configuration of a TMX TL1 Parser. */
  struct TmxTl1Configuration {

    /** Whether to log messages. */
    bool m_isLoggingMessages;

    /** The Market disseminating the data. */
    MarketCode m_market;

    /** The Country of origin. */
    Nexus::CountryCode m_country;

    /** The difference in time between the data provider's time and UTC. */
    boost::posix_time::time_duration m_timeOffset;
  };
}

#endif
