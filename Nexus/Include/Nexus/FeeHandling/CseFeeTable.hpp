#ifndef NEXUS_CSE_FEE_TABLE_HPP
#define NEXUS_CSE_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by the Canadian Securities Exchange. */
  struct CseFeeTable {

    /* Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price >= $1.00. */
      DEFAULT,

      /** Price >= $0.10 & < $1.00. */
      SUBDOLLAR,

      /** Price < $0.10. */
      SUBDIME
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(3);

    /** The fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_feeTable;
  };

  /**
   * Parses a CseFeeTable from a YAML configuration.
   * @param config The configuration to parse the CseFeeTable from.
   * @return The CseFeeTable represented by the <i>config</i>.
   */
  inline CseFeeTable ParseCseFeeTable(const YAML::Node& config) {
    auto feeTable = CseFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The CseFeeTable used to lookup the fee.
   * @param liquidityFlag The trade's LiquidityFlag.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupFee(const CseFeeTable& feeTable,
      LiquidityFlag liquidityFlag, CseFeeTable::PriceClass priceClass) {
    return feeTable.m_feeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Calculates the fee on a trade executed on CSE.
   * @param feeTable The CseFeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const CseFeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < 10 * Money::CENT) {
        return CseFeeTable::PriceClass::SUBDIME;
      } else if(executionReport.m_lastPrice < Money::ONE) {
        return CseFeeTable::PriceClass::SUBDOLLAR;
      } else {
        return CseFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto liquidityFlag = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P') {
          return LiquidityFlag::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'A') {
          return LiquidityFlag::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [CSE]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return LiquidityFlag::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [CSE]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    }();
    auto fee = LookupFee(feeTable, liquidityFlag, priceClass);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
