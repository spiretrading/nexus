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
#include "Nexus/FeeHandling/NyseFeeTable.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /*! \struct ConsolidatedUsFeeTable
      \brief Consolidates all U.S. related market fees together.
   */
  struct ConsolidatedUsFeeTable {

    //! Fee charged for the software.
    Money m_spireFee;

    //! The SEC rate.
    boost::rational<int> m_secRate;

    //! The TAF fee.
    Money m_tafFee;

    //! The NSCC rate.
    boost::rational<int> m_nsccRate;

    //! The clearing fee.
    Money m_clearingFee;

    //! Fee table used by NSDQ.
    NsdqFeeTable m_nsdqFeeTable;

    //! Fee table used by NYSE.
    NyseFeeTable m_nyseFeeTable;
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
    feeTable.m_spireFee = Beam::Extract<Money>(config, "spire_fee");
    feeTable.m_secRate = Beam::Extract<boost::rational<int>>(
      config, "sec_rate");
    feeTable.m_tafFee = Beam::Extract<Money>(config, "taf_fee");
    feeTable.m_nsccRate = Beam::Extract<boost::rational<int>>(
      config, "nscc_rate");
    feeTable.m_clearingFee = Beam::Extract<Money>(config, "clearing_fee");
    auto nasdaqConfig = config.FindValue("nasdaq");
    if(nasdaqConfig == nullptr) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error{"Fee table for NASDAQ missing."});
    } else {
      feeTable.m_nsdqFeeTable = ParseNsdqFeeTable(*nasdaqConfig);
    }
    auto nyseConfig = config.FindValue("nyse");
    if(nyseConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for NYSE missing."});
    } else {
      feeTable.m_nyseFeeTable = ParseNyseFeeTable(*nyseConfig);
    }
    return feeTable;
  }

  //! Calculates the fee on a trade executed on a U.S. market.
  /*!
    \param feeTable The ConsolidatedUsFeeTable used to calculate the fee.
    \param order The Order that was traded against.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return An ExecutionReport containing the calculated fees.
  */
  inline OrderExecutionService::ExecutionReport CalculateFee(
      const ConsolidatedUsFeeTable& feeTable,
      const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = executionReport;
    auto lastMarket = [&] {
      auto& destination = order.GetInfo().m_fields.m_destination;
      if(destination == DefaultDestinations::NASDAQ()) {
        return ToString(DefaultMarkets::NASDAQ());
      } else if(destination == DefaultDestinations::NYSE()) {
        return ToString(DefaultMarkets::NYSE());
      } else {
        return std::string{};
      }
    }();
    feesReport.m_executionFee += [&] {
      if(lastMarket == DefaultMarkets::NASDAQ()) {
        return CalculateFee(feeTable.m_nsdqFeeTable, executionReport);
      } else if(lastMarket == DefaultMarkets::NYSE()) {
        return CalculateFee(feeTable.m_nyseFeeTable, order.GetInfo().m_fields,
          executionReport);
      } else {
        std::cout << "Unknown last market [US]: \"" <<
          order.GetInfo().m_fields.m_destination << "\"\n";
        return Money::ZERO;
      }
    }();
    feesReport.m_processingFee += [&] {
      if(feesReport.m_lastQuantity != 0) {
        auto processingFee = feesReport.m_lastQuantity *
          (feeTable.m_clearingFee + feeTable.m_tafFee);
        if(order.GetInfo().m_fields.m_side == Side::BID) {
          processingFee += feeTable.m_secRate *
            (feesReport.m_lastQuantity * feesReport.m_lastPrice);
        }
        processingFee += Money::CENT + feeTable.m_nsccRate *
          (feesReport.m_lastQuantity * feesReport.m_lastPrice);
        return Ceil(processingFee, 3);
      } else {
        return Money::ZERO;
      }
    }();
    feesReport.m_commission += feesReport.m_lastQuantity * feeTable.m_spireFee;
    return feesReport;
  }
}

#endif
