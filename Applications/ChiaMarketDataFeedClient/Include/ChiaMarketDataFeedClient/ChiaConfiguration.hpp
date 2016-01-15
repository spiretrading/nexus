#ifndef NEXUS_CHIACONFIGURATION_HPP
#define NEXUS_CHIACONFIGURATION_HPP
#include <string>
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \struct ChiaConfiguration
      \brief Stores the configuration for a CHIA market data parser.
   */
  struct ChiaConfiguration {

    //! Whether to log messages.
    bool m_isLoggingMessages;

    //! The market's CountryCode.
    CountryCode m_country;

    //! The market on which the Security is listed.
    MarketCode m_primaryMarket;

    //! The Market disseminating the data.
    MarketCode m_disseminatingMarket;

    //! The MPID to display.
    std::string m_mpid;

    //! The time used as the origin of market data messages disseminated by
    //! market data server.
    boost::posix_time::ptime m_timeOrigin;

    //! Whether trades should be treated as a time and sale.
    bool m_isTimeAndSaleFeed;
  };
}
}

#endif
