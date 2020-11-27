#ifndef NEXUS_HKEX_CONFIGURATION_HPP
#define NEXUS_HKEX_CONFIGURATION_HPP
#include <string>
#include "Nexus/Definitions/Market.hpp"

namespace Nexus::MarketDataService {

  /** Stores configuration for a single HKEX feed. */
  struct HkexConfiguration {

    /** Whether to log all packets. */
    bool m_enableLogging;

    /** The market to associate the updates with. */
    MarketDatabase::Entry m_market;

    /** The MPID to use for HKEX. */
    std::string m_mpid;
  };
}

#endif
