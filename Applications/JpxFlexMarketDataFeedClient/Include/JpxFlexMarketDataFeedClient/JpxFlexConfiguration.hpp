#ifndef NEXUS_JPX_FLEX_CONFIGURATION_HPP
#define NEXUS_JPX_FLEX_CONFIGURATION_HPP
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Market.hpp"

namespace Nexus::MarketDataService {

  /** Provides the configuration of a single JPX Flex feed. */
  struct JpxFlexConfiguration {

    /** Whether to log messages. */
    bool m_enableLogging;

    /** The market represented by the feed. */
    MarketDatabase::Entry m_market;

    /** The market disseminating the data. */
    MarketCode m_disseminatingMarket;

    /** The feed's MPID. */
    std::string m_mpid;

    /** The offset from UTC of the market data's time zone. */
    boost::posix_time::time_duration m_utcOffset;
  };
}

#endif
