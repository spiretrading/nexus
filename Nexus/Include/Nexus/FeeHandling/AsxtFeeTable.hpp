#ifndef NEXUS_ASXTFEETABLE_HPP
#define NEXUS_ASXTFEETABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /*! \struct AsxtFeeTable
      \brief Stores the table of fees used by ASX TradeMatch.
   */
  struct AsxtFeeTable {

    /*! \enum PriceClass
        \brief Used to categorize fees based on the price of the security.
     */
    enum class PriceClass {

      //! Not recognized.
      NONE = -1,

      //! Price < 0.25
      TIER_ONE,

      //! 0.25 <= Price < 1.00
      TIER_TWO,

      //! Price >= 1.00
      TIER_THREE
    };

    //! The number of price classes.
    static const std::size_t PRICE_CLASS_COUNT = 3;

    //! Fee charged for the software.
    Money m_spireFee;

    //! The clearing rates.
    std::array<boost::rational<int>, PRICE_CLASS_COUNT> m_clearingRateTable;

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
    ParseFeeTable(config, "clearing_rate_table",
      Beam::Store(feeTable.m_clearingRateTable));
    feeTable.m_tradeRate = Beam::Extract<boost::rational<int>>(config,
      "trade_rate");
    feeTable.m_gstRate = Beam::Extract<boost::rational<int>>(config,
      "gst_rate");
    feeTable.m_tradeFeeCap = Beam::Extract<Money>(config, "trade_fee_cap");
    return feeTable;
  }

  //! Looks up a clearing fee.
  /*!
    \param feeTable The AsxtFeeTable used to lookup the fee.
    \param priceClass The trade's PriceClass.
    \return The fee corresponding to the specified <i>priceClass</i>.
  */
  inline boost::rational<int> LookupClearingFee(const AsxtFeeTable& feeTable,
      AsxtFeeTable::PriceClass priceClass) {
    return feeTable.m_clearingRateTable[static_cast<int>(priceClass)];
  }

  //! Looks up a trade's price class.
  /*!
    \param executionReport The execution report representing the trade.
    \return The trade's price class.
  */
  inline AsxtFeeTable::PriceClass LookupPriceClass(
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastPrice < 25 * Money::CENT) {
      return AsxtFeeTable::PriceClass::TIER_ONE;
    } else if(executionReport.m_lastPrice < Money::ONE) {
      return AsxtFeeTable::PriceClass::TIER_TWO;
    }
    return AsxtFeeTable::PriceClass::TIER_THREE;
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
    auto priceClass = LookupPriceClass(executionReport);
    auto clearingRate = LookupClearingFee(feeTable, priceClass);
    feesReport.m_processingFee += clearingRate *
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
