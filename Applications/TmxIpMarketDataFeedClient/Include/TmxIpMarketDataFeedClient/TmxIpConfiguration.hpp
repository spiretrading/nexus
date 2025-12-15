#ifndef NEXUS_TMX_IP_CONFIGURATION_HPP
#define NEXUS_TMX_IP_CONFIGURATION_HPP
#include <string>
#include <unordered_map>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores the configuration of a TMX Information Processor Parser. */
  struct TmxIpConfiguration {

    /** Whether to log messages. */
    bool m_is_logging_messages;

    /** The difference in time between the data provider's time and UTC. */
    boost::posix_time::time_duration m_time_offset;

    /** Whether trades should be treated as a time and sale. */
    bool m_is_time_and_sale_feed;

    /** The venue disseminating the data. */
    Nexus::Venue m_venue;

    /** The Country of origin. */
    Nexus::CountryCode m_country;

    /** Specifies whether the broker number is used as part of the order key. */
    bool m_use_broker_number_as_key;

    /** The default MPID to attribute Orders to. */
    std::string m_default_mpid;

    /** Whether to consolidate all Orders as originating from a single MPID. */
    bool m_consolidate_mpids;

    /** Whether the NEO book is being parsed. */
    bool m_is_neo_book;

    /** Maps native MPIDs. */
    std::unordered_map<std::string, std::string> m_mpid_mappings;
  };
}

#endif
