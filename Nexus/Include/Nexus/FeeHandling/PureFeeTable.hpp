#ifndef NEXUS_PURE_FEE_TABLE_HPP
#define NEXUS_PURE_FEE_TABLE_HPP
#include <array>
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by Pure on TSX. */
  struct PureFeeTable {

    /** Enumerates the sections of the fee tables. */
    enum class Section {

      /** Invalid section. */
      NONE = -1,

      /** The default fee table. */
      DEFAULT,

      /** Applies to interlisted securities. */
      INTERLISTED,

      /** Applies to ETFs. */
      ETF
    };

    /** The number of sections enumerated. */
    static const auto SECTION_COUNT = std::size_t(3);

    /** Enumerates the rows of each section. */
    enum class Row {

      /** Invalid row. */
      NONE = -1,

      /** The row of sub-dollar fees. */
      SUBDOLLAR,

      /** The row of default fees. */
      DEFAULT,

      /** The row of dark sub-dollar fees. */
      DARK_SUBDOLLAR,

      /** The row of dark fees. */
      DARK
    };

    /** The number of price classes enumerated. */
    static constexpr auto ROW_COUNT = std::size_t(4);

    /** The fee table. */
    std::array<std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, ROW_COUNT>,
      SECTION_COUNT> m_feeTable;
  };

  /**
   * Parses a PureFeeTable from a YAML configuration.
   * @param config The configuration to parse the PureFeeTable from.
   * @return The PureFeeTable represented by the <i>config</i>.
   */
  inline PureFeeTable ParsePureFeeTable(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    auto feeTable = PureFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(
      feeTable.m_feeTable[static_cast<int>(PureFeeTable::Section::DEFAULT)]));
    ParseFeeTable(config, "interlisted_table", Beam::Store(feeTable.m_feeTable[
      static_cast<int>(PureFeeTable::Section::INTERLISTED)]));
    ParseFeeTable(config, "etf_table", Beam::Store(feeTable.m_feeTable[
      static_cast<int>(PureFeeTable::Section::ETF)]));
    return feeTable;
  }

  /**
   * Returns the row to use in a PureFeeTable based on an execution report.
   * @param executionReport The execution report to get the row for.
   */
  inline PureFeeTable::Row LookupPureRow(
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_liquidityFlag.size() >= 3 &&
        executionReport.m_liquidityFlag[2] == 'D') {
      if(executionReport.m_lastPrice < Money::ONE) {
        return PureFeeTable::Row::DARK_SUBDOLLAR;
      }
      return PureFeeTable::Row::DARK;
    } else if(executionReport.m_lastPrice < Money::ONE) {
      return PureFeeTable::Row::SUBDOLLAR;
    }
    return PureFeeTable::Row::DEFAULT;
  }

  /**
   * Returns the liquidity flag based on a Pure execution.
   * @param executionReport The execution report to get the liquidity flag for.
   */
  inline LiquidityFlag LookupPureLiquidityFlag(
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(!executionReport.m_liquidityFlag.empty()) {
      if(executionReport.m_liquidityFlag[0] == 'P') {
        return LiquidityFlag::PASSIVE;
      } else if(executionReport.m_liquidityFlag[0] == 'T') {
        return LiquidityFlag::ACTIVE;
      }
    }
    std::cout << "Unknown liquidity flag [PURE]: \"" <<
      executionReport.m_liquidityFlag << "\"\n";
    return LiquidityFlag::ACTIVE;
  }

  /**
   * Looks up a fee in the PureFeeTable.
   * @param feeTable The PureFeeTable used to lookup the fee.
   * @param section The section of the fee table to lookup.
   * @param row The row of the table to lookup.
   * @param liquidityFlag The liquidity flag to lookup.
   * @return The fee corresponding to the specified <i>section</i>, <i>row</i>,
   *         and <i>liquidityFlag</i>.
   */
  inline Money LookupFee(const PureFeeTable& feeTable,
      PureFeeTable::Section section, PureFeeTable::Row row,
      LiquidityFlag liquidityFlag) {
    return feeTable.m_feeTable[static_cast<int>(section)][
      static_cast<int>(row)][static_cast<int>(liquidityFlag)];
  }

  /**
   * Calculates the fee on a trade executed on PURE.
   * @param feeTable The PureFeeTable used to calculate the fee.
   * @param section The section of the table to lookup.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(
      const PureFeeTable& feeTable, PureFeeTable::Section section,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto row = LookupPureRow(executionReport);
    auto liquidityFlag = LookupPureLiquidityFlag(executionReport);
    auto fee = LookupFee(feeTable, section, row, liquidityFlag);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
