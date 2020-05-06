#ifndef NEXUS_MARKETDATAFEEDSERVICES_HPP
#define NEXUS_MARKETDATAFEEDSERVICES_HPP
#include <Beam/Serialization/ShuttleBitset.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus {
namespace MarketDataService {
  using MarketDataFeedMessage = boost::variant<SecurityBboQuote,
    SecurityBookQuote, SecurityMarketQuote, SecurityTimeAndSale,
    MarketOrderImbalance>;

  BEAM_DEFINE_MESSAGES(MarketDataFeedMessages,

    /*! \interface Nexus::MarketDataService::ResetDataMessage
        \brief Resets specified data.
        \param messageTypes The type of data to reset.
    */
    //! \cond
    (ResetDataMessage, "Nexus.MarketDataService.ResetDataMessage",
      MarketDataTypeSet, messageTypes),

    /*! \interface Nexus::MarketDataService::SetSecurityInfoMessage
        \brief Sets or updates a SecurityInfo.
        \param security_info The updated SecurityInfo.
    */
    //! \cond
    (SetSecurityInfoMessage, "Nexus.MarketDataService.SetSecurityInfoMessage",
      SecurityInfo, security_info),

    /*! \interface Nexus::MarketDataService::SendMarketDataFeedMessages
        \brief Sends a list of MarketDataFeedMessages for processing.
        \param messages The list of MarketDataFeedMessages to send.
    */
    //! \cond
    (SendMarketDataFeedMessages,
      "Nexus.MarketDataService.SendMarketDataFeedMessages",
      std::vector<MarketDataFeedMessage>, messages));
    //! \endcond
}
}

#endif
