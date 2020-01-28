#ifndef NEXUS_HKEX_CONFIGURATION_HPP
#define NEXUS_HKEX_CONFIGURATION_HPP
#include "Nexus/Definitions/Market.hpp"

namespace Nexus::MarketDataService {
  struct HkexConfiguration {
    bool m_enableLogging;
    MarketDatabase::Entry m_market;
    MarketCode m_disseminatingMarket;
    std::string m_mpid;
  };
}

#endif
