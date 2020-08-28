#ifndef NEXUS_LYNX_FEE_TABLE_HPP
#define NEXUS_LYNX_FEE_TABLE_HPP
#include <array>
#include <exception>
#include <unordered_map>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by the LYNX ATS. */
  struct LynxFeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price >= $1.00. */
      DEFAULT = 0,

      /** Price < $1.00. */
      SUBDOLLAR,
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(2);

    /** The fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_feeTable;
  };

  /**
   * Parses a LynxFeeTable from a YAML configuration.
   * @param config The configuration to parse the LynxFeeTable from.
   * @return The LynxFeeTable represented by the <i>config</i>.
   */
  inline LynxFeeTable ParseLynxFeeTable(const YAML::Node& config) {
    auto feeTable = LynxFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The LynxFeeTable used to lookup the fee.
   * @param liquidityFlag The liquidity flag to lookup.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag, LynxFeeTable::PriceClass priceClass) {
    return feeTable.m_feeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Calculates the fee on a trade executed on LYNX.
   * @param feeTable The LynxFeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const LynxFeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto liquidityFlag = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P') {
          return LiquidityFlag::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'A') {
          return LiquidityFlag::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [LYNX]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return LiquidityFlag::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [LYNX]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    }();
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < Money::ONE) {
        return LynxFeeTable::PriceClass::SUBDOLLAR;
      } else {
        return LynxFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto fee = LookupFee(feeTable, liquidityFlag, priceClass);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
