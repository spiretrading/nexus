#ifndef NEXUS_TMX_IP_CONFIGURATION_HPP
#define NEXUS_TMX_IP_CONFIGURATION_HPP
#include <string>
#include <unordered_map>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus::MarketDataService {

  /** Stores the configuration of a TMX Information Processor Parser. */
  struct TmxIpConfiguration {

    /** Whether to log messages. */
    bool m_isLoggingMessages;

    /** The difference in time between the data provider's time and UTC. */
    boost::posix_time::time_duration m_timeOffset;

    /** Whether trades should be treated as a time and sale. */
    bool m_isTimeAndSaleFeed;

    /** The Market disseminating the data. */
    Nexus::MarketCode m_market;

    /** The Country of origin. */
    Nexus::CountryCode m_country;

    /** Specifies whether the broker number is used as part of the order key. */
    bool m_useBrokerNumberAsKey;

    /** The default MPID to attribute Orders to. */
    std::string m_defaultMpid;

    /** Whether to consolidate all Orders as originating from a single MPID. */
    bool m_consolidateMpids;

    /** Whether the NEO book is being parsed. */
    bool m_isNeoBook;

    /** Maps native MPIDs. */
    std::unordered_map<std::string, std::string> m_mpidMappings;
  };
}

#endif
