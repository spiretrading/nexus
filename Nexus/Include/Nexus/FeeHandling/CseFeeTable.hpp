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

    /** Enumerates the types of fees that can be applied. */
    enum class Section {

      /** Unknown. */
      NONE = -1,

      /** Lit order with price greater than or equal to $1.00. */
      DEFAULT,

      /** Lit order with price less than $1.00. */
      SUBDOLLAR,

      /** Dark order. */
      DARK
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
   * Determines what section of the CSE fee table is needed to calculate a fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The section within the <i>CseFeeTable</i> needed to calculate the
   *         fee for the specified <i>executionReport</i>.
   */
  inline CseFeeTable::Section LookupCseFeeTableSection(
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_liquidityFlag.size() >= 3 &&
        executionReport.m_liquidityFlag[2] == 'D') {
      return CseFeeTable::Section::DARK;
    } else if(executionReport.m_lastPrice < Money::ONE) {
      return CseFeeTable::Section::SUBDOLLAR;
    }
    return CseFeeTable::Section::DEFAULT;
  }

  /**
   * Returns the liquidity flag assigned to an execution report.
   * @param executionReport The ExecutionReport to get the liquidity flag for.
   * @return The liquidity flag assigned to the <i>executionReport</i>.
   */
  inline LiquidityFlag LookupCseLiquidityFlag(
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_liquidityFlag.size() >= 1) {
      if(executionReport.m_liquidityFlag[0] == 'P') {
        return LiquidityFlag::PASSIVE;
      } else if(executionReport.m_liquidityFlag[0] == 'T') {
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
  }

  /**
   * Looks up a fee in the CseFeeTable.
   * @param feeTable The CseFeeTable used to lookup the fee.
   * @param liquidityFlag The trade's LiquidityFlag.
   * @param section The section of the fee table to lookup.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>section</i>.
   */
  inline Money LookupFee(const CseFeeTable& feeTable,
      LiquidityFlag liquidityFlag, CseFeeTable::Section section) {
    return feeTable.m_feeTable[static_cast<int>(section)][
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
    auto liquidityFlag = LookupCseLiquidityFlag(executionReport);
    auto section = LookupCseFeeTableSection(executionReport);
    auto fee = LookupFee(feeTable, liquidityFlag, section);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
