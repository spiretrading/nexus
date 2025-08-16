#ifndef NEXUS_CSE_FEE_TABLE_HPP
#define NEXUS_CSE_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
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
    static constexpr auto SECTION_COUNT = std::size_t(3);

    /** The fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, SECTION_COUNT>
      m_fee_table;
  };

  /**
   * Parses a CseFeeTable from a YAML configuration.
   * @param config The configuration to parse the CseFeeTable from.
   * @return The CseFeeTable represented by the <i>config</i>.
   */
  inline CseFeeTable ParseCseFeeTable(const YAML::Node& config) {
    auto table = CseFeeTable();
    parse_fee_table(config, "fee_table", Beam::Store(table.m_fee_table));
    return table;
  }

  /**
   * Determines what section of the CSE fee table is needed to calculate a fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The section within the <i>CseFeeTable</i> needed to calculate the
   *         fee for the specified <i>report</i>.
   */
  inline CseFeeTable::Section lookup_cse_fee_table_section(
      const OrderExecutionService::ExecutionReport& report) {
    if(report.m_liquidity_flag.size() >= 3 &&
        report.m_liquidity_flag[2] == 'D') {
      return CseFeeTable::Section::DARK;
    } else if(report.m_last_price < Money::ONE) {
      return CseFeeTable::Section::SUBDOLLAR;
    }
    return CseFeeTable::Section::DEFAULT;
  }

  /**
   * Returns the liquidity flag assigned to an execution report.
   * @param report The ExecutionReport to get the liquidity flag for.
   * @return The liquidity flag assigned to the <i>report</i>.
   */
  inline LiquidityFlag lookup_cse_liquidity_flag(
      const OrderExecutionService::ExecutionReport& report) {
    if(report.m_liquidity_flag.size() >= 1) {
      if(report.m_liquidity_flag[0] == 'P') {
        return LiquidityFlag::PASSIVE;
      } else if(report.m_liquidity_flag[0] == 'T') {
        return LiquidityFlag::ACTIVE;
      } else {
        std::cout << "Unknown liquidity flag [CSE]: \"" <<
          report.m_liquidity_flag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    } else {
      std::cout << "Unknown liquidity flag [CSE]: \"" <<
        report.m_liquidity_flag << "\"\n";
      return LiquidityFlag::ACTIVE;
    }
  }

  /**
   * Looks up a fee in the CseFeeTable.
   * @param table The CseFeeTable used to lookup the fee.
   * @param flag The trade's LiquidityFlag.
   * @param section The section of the fee table to lookup.
   * @return The fee corresponding to the specified <i>flag</i> and
   *         <i>section</i>.
   */
  inline Money lookup_fee(const CseFeeTable& table, LiquidityFlag flag,
      CseFeeTable::Section section) {
    return table.m_fee_table[static_cast<int>(section)][
      static_cast<int>(flag)];
  }

  /**
   * Calculates the fee on a trade executed on CSE.
   * @param table The CseFeeTable used to calculate the fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(const CseFeeTable& table,
      const OrderExecutionService::ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto flag = lookup_cse_liquidity_flag(report);
    auto section = lookup_cse_fee_table_section(report);
    auto fee = lookup_fee(table, flag, section);
    return report.m_last_quantity * fee;
  }
}

#endif
