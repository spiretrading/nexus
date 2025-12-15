#ifndef NEXUS_CSE_CONFIGURATION_HPP
#define NEXUS_CSE_CONFIGURATION_HPP
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores the configuration of a CSE data feed. */
  struct CseConfiguration {

    /** Whether to log messages. */
    bool m_is_logging_messages;

    /** Whether to treat trade messages as a time and sale. */
    bool m_is_time_and_sale_feed;

    /** The difference in time between the data provider's time and UTC. */
    boost::posix_time::time_duration m_time_offset;

    /** The venue symbols are assigned to. */
    Venue m_venue;

    /** The set of Securities listed on the market. */
    std::unordered_set<std::string> m_securities;

    /** Maps native MPIDs. */
    std::unordered_map<std::string, std::string> m_mpid_mappings;
  };
}

#endif
