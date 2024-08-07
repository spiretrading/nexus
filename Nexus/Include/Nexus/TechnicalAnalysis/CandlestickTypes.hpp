#ifndef NEXUS_CANDLESTICK_TYPES_HPP
#define NEXUS_CANDLESTICK_TYPES_HPP
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Nexus/TechnicalAnalysis/TechnicalAnalysis.hpp"

namespace Nexus::TechnicalAnalysis {

  /** Defines a Candlestick representing a price at a point in time. */
  using TimePriceCandlestick = Candlestick<boost::posix_time::ptime, Money>;

  /** Defines a time/price series. */
  using TimePriceSeries = std::vector<TimePriceCandlestick>;
}

#endif
