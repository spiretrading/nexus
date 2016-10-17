#ifndef NEXUS_CONSOLIDATEDUSFEETABLE_HPP
#define NEXUS_CONSOLIDATEDUSFEETABLE_HPP
#include <exception>
#include <sstream>
#include <unordered_set>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/NsdqFeeTable.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /*! \struct ConsolidatedUsFeeTable
      \brief Consolidates all U.S. related market fees together.
   */
  struct ConsolidatedUsFeeTable {

    //! Fee table used by NSDQ.
    NsdqFeeTable m_nsdqFeeTable;
  };

  //! Parses a ConsolidatedUsFeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the ConsolidatedUsFeeTable from.
    \param marketDatabase The MarketDatabase used to parse Securities.
    \return The ConsolidatedUsFeeTable represented by the <i>config</i>.
  */
  inline ConsolidatedUsFeeTable ParseConsolidatedUsFeeTable(
      const YAML::Node& config, const MarketDatabase& marketDatabase) {
    ConsolidatedUsFeeTable feeTable;
    auto nasdaqConfig = config.FindValue("nasdaq");
    if(nasdaqConfig == nullptr) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error{"Fee table for NASDAQ missing."});
    } else {
      feeTable.m_nsdqFeeTable = ParseNsdqFeeTable(*nasdaqConfig);
    }
    return feeTable;
  }

  //! Calculates the fee on a trade executed on a U.S. market.
  /*!
    \param feeTable The ConsolidatedUsFeeTable used to calculate the fee.
    \param order The Order that was traded against.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return The fee calculated for the specified trade.
  */
  inline Money CalculateFee(const ConsolidatedUsFeeTable& feeTable,
      const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto lastMarket = [&] {
      auto& destination = order.GetInfo().m_fields.m_destination;
      if(destination == DefaultDestinations::NASDAQ()) {
        return ToString(DefaultMarkets::NASDAQ());
      } else {
        return std::string{};
      }
    }();
    if(lastMarket == DefaultMarkets::NASDAQ()) {
      return CalculateFee(feeTable.m_nsdqFeeTable, executionReport);
    } else {
      std::cout << "Unknown last market [US]: \"" <<
        order.GetInfo().m_fields.m_destination << "\"";
      return Money::ZERO;
    }
  }
}

#endif
