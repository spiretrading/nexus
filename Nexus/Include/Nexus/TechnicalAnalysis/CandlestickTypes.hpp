#ifndef NEXUS_CANDLESTICK_TYPES_HPP
#define NEXUS_CANDLESTICK_TYPES_HPP
#include <vector>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"

namespace Nexus {

  /** Defines a Candlestick representing a price at a point in time. */
  using PriceCandlestick = Candlestick<boost::posix_time::ptime, Money>;

  /** Defines a time/price series. */
  using PriceSeries = std::vector<PriceCandlestick>;
}

#endif
