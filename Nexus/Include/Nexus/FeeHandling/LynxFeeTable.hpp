#ifndef NEXUS_LYNX_FEE_TABLE_HPP
#define NEXUS_LYNX_FEE_TABLE_HPP
#include <array>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by the LYNX ATS. */
  struct LynxFeeTable {

    /** Lists the available trade classifications. */
    enum class Classification {

      /** Unknown. */
      NONE = -1,

      /** Price >= $1.00 and not interlisted. */
      DEFAULT,

      /** Price >= $1.00 and interlisted. */
      INTERLISTED,

      /** ETFs. */
      ETF,

      /** Price < $1.00. */
      SUBDOLLAR,

      /** Midpoint execution. */
      MIDPOINT
    };

    /** The number of price classes enumerated. */
    static constexpr auto CLASSIFICATION_COUNT = std::size_t(5);

    /** The fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, CLASSIFICATION_COUNT>
      m_fee_table;

    /** The set of interlisted tickers. */
    std::unordered_set<Ticker> m_interlisted;

    /** The set of ETFs. */
    std::unordered_set<Ticker> m_etfs;
  };

  /**
   * Parses a LynxFeeTable from a YAML configuration.
   * @param config The configuration to parse the LynxFeeTable from.
   * @param interlisted The set of interlisted Tickers.
   * @param etfs The set of ETF Tickers.
   * @return The LynxFeeTable represented by the <i>config</i>.
   */
  inline LynxFeeTable parse_lynx_fee_table(const YAML::Node& config,
      std::unordered_set<Ticker> etfs, std::unordered_set<Ticker> interlisted) {
    auto table = LynxFeeTable();
    parse_fee_table(config, "fee_table", Beam::out(table.m_fee_table));
    table.m_interlisted = std::move(interlisted);
    table.m_etfs = std::move(etfs);
    return table;
  }

  /**
   * Tests if an OrderFields represents a midpoint order on LYNX.
   * @param fields The OrderFields to test.
   * @return <code>true</code> iff the <i>order</i> counts as a midpoint order
   * on LYNX.
   */
  inline bool is_lynx_midpoint_order(const OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      has_field(fields, Tag(18, "M"));
  }

  /**
   * Looks up a fee.
   * @param table The LynxFeeTable used to lookup the fee.
   * @param flag The liquidity flag to lookup.
   * @param classification The trade's classification.
   * @return The fee corresponding to the specified <i>flag</i> and
   *         <i>price_class</i>.
   */
  inline Money lookup_fee(const LynxFeeTable& table, LiquidityFlag flag,
      LynxFeeTable::Classification classification) {
    return table.m_fee_table[static_cast<int>(classification)][
      static_cast<int>(flag)];
  }

  /**
   * Calculates the fee on a trade executed on LYNX.
   * @param table The LynxFeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(const LynxFeeTable& table,
      const OrderFields& fields, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto flag = [&] {
      if(report.m_liquidity_flag.size() == 1) {
        if(report.m_liquidity_flag[0] == 'P') {
          return LiquidityFlag::PASSIVE;
        } else if(report.m_liquidity_flag[0] == 'A') {
          return LiquidityFlag::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [LYNX]: \"" <<
            report.m_liquidity_flag << "\"\n";
          return LiquidityFlag::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [LYNX]: \"" <<
          report.m_liquidity_flag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    }();
    auto classification = [&] {
      if(is_lynx_midpoint_order(fields)) {
        return LynxFeeTable::Classification::MIDPOINT;
      } else if(report.m_last_price < Money::ONE) {
        return LynxFeeTable::Classification::SUBDOLLAR;
      } else if(table.m_interlisted.count(fields.m_ticker) == 1) {
        return LynxFeeTable::Classification::INTERLISTED;
      } else if(table.m_etfs.count(fields.m_ticker) == 1) {
        return LynxFeeTable::Classification::ETF;
      } else {
        return LynxFeeTable::Classification::DEFAULT;
      }
    }();
    auto fee = lookup_fee(table, flag, classification);
    return report.m_last_quantity * fee;
  }
}

#endif
