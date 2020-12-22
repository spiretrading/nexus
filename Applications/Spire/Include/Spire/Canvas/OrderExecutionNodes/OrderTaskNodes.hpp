#ifndef SPIRE_ORDERTASKNODES_HPP
#define SPIRE_ORDERTASKNODES_HPP
#include <memory>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Returns a SingleOrderTaskNode with an ASK Side.
  std::unique_ptr<SingleOrderTaskNode> GetAskOrderTaskNode();

  //! Returns a SingleOrderTaskNode with an BID Side.
  std::unique_ptr<SingleOrderTaskNode> GetBidOrderTaskNode();

  //! Returns a SingleOrderTaskNode with a LIMIT OrderType.
  std::unique_ptr<SingleOrderTaskNode> GetLimitOrderTaskNode();

  //! Returns a SingleOrderTaskNode with a LIMIT OrderType and ASK Side.
  std::unique_ptr<SingleOrderTaskNode> GetLimitAskOrderTaskNode();

  //! Returns a SingleOrderTaskNode with a LIMIT OrderType and BID Side.
  std::unique_ptr<SingleOrderTaskNode> GetLimitBidOrderTaskNode();

  //! Returns a SingleOrderTaskNode with a MARKET OrderType.
  std::unique_ptr<SingleOrderTaskNode> GetMarketOrderTaskNode();

  //! Returns a SingleOrderTaskNode with a MARKET OrderType and ASK Side.
  std::unique_ptr<SingleOrderTaskNode> GetMarketAskOrderTaskNode();

  //! Returns a SingleOrderTaskNode with a MARKET OrderType and BID Side.
  std::unique_ptr<SingleOrderTaskNode> GetMarketBidOrderTaskNode();

  //! Returns a SingleOrderTaskNode with a PEGGED OrderType.
  /*!
    \param isPriceOptional Whether the price parameter is optional.
  */
  std::unique_ptr<SingleOrderTaskNode> GetPeggedOrderTaskNode(
    bool isPriceOptional);

  //! Returns a SingleOrderTaskNode with a PEGGED OrderType and ASK Side.
  /*!
    \param optionalPrice Whether the price parameter is optional.
  */
  std::unique_ptr<SingleOrderTaskNode> GetPeggedAskOrderTaskNode(
    bool isPriceOptional);

  //! Returns a SingleOrderTaskNode with a PEGGED OrderType and BID Side.
  /*!
    \param optionalPrice Whether the price parameter is optional.
  */
  std::unique_ptr<SingleOrderTaskNode> GetPeggedBidOrderTaskNode(
    bool isPriceOptional);

  //! Returns a SingleOrderTaskNode representing an OrderField's instance.
  /*!
    \param orderFields The OrderFields to build the CanvasNode from.
    \param userProfile The user's profile.
  */
  std::unique_ptr<SingleOrderTaskNode> MakeOrderTaskNodeFromOrderFields(
    const Nexus::OrderExecutionService::OrderFields& orderFields,
    const UserProfile& userProfile);
}

#endif
