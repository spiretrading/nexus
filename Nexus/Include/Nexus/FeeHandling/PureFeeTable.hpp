#ifndef NEXUS_PURE_FEE_TABLE_HPP
#define NEXUS_PURE_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
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
      SECTION_COUNT> m_fee_table;
  };

  /**
   * Parses a PureFeeTable from a YAML configuration.
   * @param config The configuration to parse the PureFeeTable from.
   * @return The PureFeeTable represented by the <i>config</i>.
   */
  inline PureFeeTable parse_pure_fee_table(const YAML::Node& config) {
    auto table = PureFeeTable();
    parse_fee_table(config, "fee_table", Beam::out(
      table.m_fee_table[static_cast<int>(PureFeeTable::Section::DEFAULT)]));
    parse_fee_table(config, "interlisted_table", Beam::out(
      table.m_fee_table[static_cast<int>(PureFeeTable::Section::INTERLISTED)]));
    parse_fee_table(config, "etf_table", Beam::out(
      table.m_fee_table[static_cast<int>(PureFeeTable::Section::ETF)]));
    return table;
  }

  /**
   * Returns the row to use in a PureFeeTable based on an execution report.
   * @param report The execution report to get the row for.
   */
  inline PureFeeTable::Row lookup_pure_row(const ExecutionReport& report) {
    if(report.m_liquidity_flag.size() >= 3 &&
        report.m_liquidity_flag[2] == 'D') {
      if(report.m_last_price < Money::ONE) {
        return PureFeeTable::Row::DARK_SUBDOLLAR;
      }
      return PureFeeTable::Row::DARK;
    } else if(report.m_last_price < Money::ONE) {
      return PureFeeTable::Row::SUBDOLLAR;
    }
    return PureFeeTable::Row::DEFAULT;
  }

  /**
   * Returns the liquidity flag based on a Pure execution.
   * @param report The execution report to get the liquidity flag for.
   */
  inline LiquidityFlag lookup_pure_liquidity_flag(
      const ExecutionReport& report) {
    if(!report.m_liquidity_flag.empty()) {
      if(report.m_liquidity_flag[0] == 'P') {
        return LiquidityFlag::PASSIVE;
      } else if(report.m_liquidity_flag[0] == 'T') {
        return LiquidityFlag::ACTIVE;
      }
    }
    std::cout << "Unknown liquidity flag [PURE]: \"" <<
      report.m_liquidity_flag << "\"\n";
    return LiquidityFlag::ACTIVE;
  }

  /**
   * Looks up a fee in the PureFeeTable.
   * @param table The PureFeeTable used to lookup the fee.
   * @param section The section of the fee table to lookup.
   * @param row The row of the table to lookup.
   * @param flag The liquidity flag to lookup.
   * @return The fee corresponding to the specified <i>section</i>, <i>row</i>,
   *         and <i>flag</i>.
   */
  inline Money lookup_fee(const PureFeeTable& table,
      PureFeeTable::Section section, PureFeeTable::Row row,
      LiquidityFlag flag) {
    return table.m_fee_table[static_cast<int>(section)][
      static_cast<int>(row)][static_cast<int>(flag)];
  }

  /**
   * Calculates the fee on a trade executed on PURE.
   * @param table The PureFeeTable used to calculate the fee.
   * @param section The section of the table to lookup.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(const PureFeeTable& table,
      PureFeeTable::Section section, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto row = lookup_pure_row(report);
    auto flag = lookup_pure_liquidity_flag(report);
    auto fee = lookup_fee(table, section, row, flag);
    return report.m_last_quantity * fee;
  }
}

#endif
