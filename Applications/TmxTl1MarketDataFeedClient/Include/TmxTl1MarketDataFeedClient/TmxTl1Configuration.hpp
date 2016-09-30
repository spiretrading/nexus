#ifndef NEXUS_TMXTL1CONFIGURATION_HPP
#define NEXUS_TMXTL1CONFIGURATION_HPP
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \struct TmxTl1Configuration
      \brief Stores the configuration of a TMX TL1 Parser.
   */
  struct TmxTl1Configuration {

    //! Whether to log messages.
    bool m_isLoggingMessages;

    //! The Market disseminating the data.
    Nexus::MarketCode m_market;

    //! The Country of origin.
    Nexus::CountryCode m_country;

    //! The difference in time between the data provider's time and UTC.
    boost::posix_time::time_duration m_timeOffset;
  };
}
}

#endif
