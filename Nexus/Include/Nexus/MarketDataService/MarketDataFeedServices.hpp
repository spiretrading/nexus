#ifndef NEXUS_MARKET_DATA_FEED_SERVICES_HPP
#define NEXUS_MARKET_DATA_FEED_SERVICES_HPP
#include <string>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus {
  using MarketDataFeedMessage = boost::variant<SecurityBboQuote,
    SecurityBookQuote, SecurityTimeAndSale, VenueOrderImbalance>;

  /** Standard name for the market data feed service. */
  inline const auto MARKET_DATA_FEED_SERVICE_NAME =
    std::string("market_data_feed_service");

  BEAM_DEFINE_MESSAGES(market_data_feed_messages,

    /**
     * Sets or updates a SecurityInfo.
     * @param security_info The updated SecurityInfo.
     */
    (SetSecurityInfoMessage, "Nexus.MarketDataService.SetSecurityInfoMessage",
      (SecurityInfo, security_info)),

    /**
     * Sends a list of MarketDataFeedMessages for processing.
     * @param messages The list of MarketDataFeedMessages to send.
     */
    (SendMarketDataFeedMessages,
      "Nexus.MarketDataService.SendMarketDataFeedMessages",
      (std::vector<MarketDataFeedMessage>, messages)));
}

#endif
