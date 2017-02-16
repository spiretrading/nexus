#ifndef NEXUS_ASXTFEETABLE_HPP
#define NEXUS_ASXTFEETABLE_HPP
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /*! \struct AsxtFeeTable
      \brief Stores the table of fees used by ASX TradeMatch.
   */
  struct AsxtFeeTable {

    //! Fee charged for the software.
    Money m_spireFee;

    //! The clearing fee.
    boost::rational<int> m_clearingRate;

    //! The equities trade rate.
    boost::rational<int> m_tradeRate;

    //! The GST rate.
    boost::rational<int> m_gstRate;

    //! The cap on the trade fee.
    Money m_tradeFeeCap;
  };

  //! Parses an AsxtFeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the AsxtFeeTable from.
    \return The AsxtFeeTable represented by the <i>config</i>.
  */
  inline AsxtFeeTable ParseAsxFeeTable(const YAML::Node& config) {
    AsxtFeeTable feeTable;
    feeTable.m_spireFee = Beam::Extract<Money>(config, "spire_fee");
    feeTable.m_clearingRate = Beam::Extract<boost::rational<int>>(
      config, "clearing_rate");
    feeTable.m_tradeRate = Beam::Extract<boost::rational<int>>(config,
      "trade_rate");
    feeTable.m_gstRate = Beam::Extract<boost::rational<int>>(config,
      "gst_rate");
    feeTable.m_tradeFeeCap = Beam::Extract<Money>(config, "trade_fee_cap");
    return feeTable;
  }

  //! Calculates the fee on a trade executed on ASXT.
  /*!
    \param feeTable The AsxtFeeTable used to calculate the fee.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return An ExecutionReport containing the calculated fees.
  */
  inline OrderExecutionService::ExecutionReport CalculateFee(
      const AsxtFeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = executionReport;
    feesReport.m_processingFee += feeTable.m_clearingRate *
      (feesReport.m_lastQuantity * feesReport.m_lastPrice);
    if(feesReport.m_lastQuantity != 0) {
      feesReport.m_commission += feeTable.m_spireFee;
    }
    feesReport.m_executionFee += [&] {
      if(executionReport.m_lastQuantity == 0) {
        return Money::ZERO;
      }
      auto notionalValue = executionReport.m_lastQuantity *
        executionReport.m_lastPrice;
      auto baseTradeFee = [&] () -> Money {
        if(feeTable.m_tradeFeeCap == Money::ZERO) {
          return feeTable.m_tradeRate * notionalValue;
        } else {
          return std::min(feeTable.m_tradeRate * notionalValue,
            feeTable.m_tradeFeeCap);
        }
      }();
      auto tradeFee = (1 + feeTable.m_gstRate) * baseTradeFee;
      return tradeFee;
    }();
    return feesReport;
  }
}

#endif
