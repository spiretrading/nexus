#ifndef NEXUS_NEOE_CONFIGURATION_HPP
#define NEXUS_NEOE_CONFIGURATION_HPP
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Nexus::MarketDataService {

  /** Stores the configuration of a NEOE data feed. */
  struct NeoeConfiguration {

    /** Whether to log messages. */
    bool m_isLoggingMessages;

    /** Whether to treat trade messages as a time and sale. */
    bool m_isTimeAndSaleFeed;

    /** The difference in time between the data provider's time and UTC. */
    boost::posix_time::time_duration m_timeOffset;

    /** The set of Securities listed on the market. */
    std::unordered_set<std::string> m_securities;

    /** Maps native MPIDs. */
    std::unordered_map<std::string, std::string> m_mpidMappings;
  };
}

#endif
