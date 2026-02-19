#ifndef NEXUS_XCX2_FEE_TABLE_HPP
#define NEXUS_XCX2_FEE_TABLE_HPP
#include <array>
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by CX2 on TSX. */
  struct Xcx2FeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price >= $5.00. */
      DEFAULT,

      /** ETFs ($1.00 or over). */
      ETF,

      /** Price < $5.00. */
      SUB_FIVE_DOLLAR,

      /** Price < $1.00. */
      SUBDOLLAR,

      /** Price < $0.10. */
      SUBDIME,
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(5);

    /** Enumerates the types of trades. */
    enum class Type {

      /** Unknown. */
      NONE = -1,

      /** Active. */
      ACTIVE = 0,

      /** Passive. */
      PASSIVE,

      /** Large Size Trade Active. */
      LARGE_ACTIVE,

      /** Large Size Trade Passive. */
      LARGE_PASSIVE,

      /** Hidden Active. */
      HIDDEN_ACTIVE,

      /** Hidden Passive. */
      HIDDEN_PASSIVE,

      /** Odd-lot. */
      ODD_LOT
    };

    /** The number of trade types enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(7);

    /** The fee table for non-TSX listed securities. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT>
      m_default_table;

    /** The fee table for TSX listed securities. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT> m_tsx_table;

    /** The large trade size threshold. */
    Quantity m_large_trade_size;

    /** The set of ETFs. */
    std::unordered_set<Ticker> m_etfs;
  };

  /**
   * Parses an Xcx2FeeTable from a YAML configuration.
   * @param config The configuration to parse the Xcx2FeeTable from.
   * @param etfs The set of ETF Tickers.
   * @return The Xcx2FeeTable represented by the <i>config</i>.
   */
  inline Xcx2FeeTable parse_xcx2_fee_table(
      const YAML::Node& config, std::unordered_set<Ticker> etfs) {
    auto table = Xcx2FeeTable();
    parse_fee_table(config, "default_table", Beam::out(table.m_default_table));
    parse_fee_table(config, "tsx_table", Beam::out(table.m_tsx_table));
    table.m_large_trade_size =
      Beam::extract<Quantity>(config, "large_trade_size");
    table.m_etfs = std::move(etfs);
    return table;
  }

  /**
   * Looks up a fee.
   * @param table The Xcx2FeeTable used to lookup the fee.
   * @param fields The OrderFields the trade took place on.
   * @param type The trade's Type.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>type</i> and
   *         <i>price_class</i>.
   */
  inline Money lookup_fee(const Xcx2FeeTable& table, const OrderFields& fields,
      Xcx2FeeTable::Type type, Xcx2FeeTable::PriceClass price_class) {
    if(fields.m_ticker.get_venue() == DefaultVenues::TSX) {
      return table.m_tsx_table[static_cast<int>(price_class)][
        static_cast<int>(type)];
    } else {
      return table.m_default_table[static_cast<int>(price_class)][
        static_cast<int>(type)];
    }
  }

  /**
   * Calculates the fee on a trade executed on XCX2.
   * @param table The Xcx2FeeTable used to calculate the fee.
   * @param fields The OrderFields the trade took place on.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(const Xcx2FeeTable& table,
      const OrderFields& fields, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto price_class = [&] {
      if(table.m_etfs.contains(fields.m_ticker)) {
        return Xcx2FeeTable::PriceClass::ETF;
      } else if(report.m_last_price < 10 * Money::CENT) {
        return Xcx2FeeTable::PriceClass::SUBDIME;
      } else if(report.m_last_price < Money::ONE) {
        return Xcx2FeeTable::PriceClass::SUBDOLLAR;
      } else if (report.m_last_price < 5 * Money::ONE) {
        return Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR;
      } else {
        return Xcx2FeeTable::PriceClass::DEFAULT;
      }
    }();
    auto type = [&] {
      if(report.m_last_quantity < 100) {
        return Xcx2FeeTable::Type::ODD_LOT;
      } else if(report.m_liquidity_flag.size() == 1) {
        if(report.m_liquidity_flag[0] == 'P' ||
            report.m_liquidity_flag[0] == 'S') {
          if(report.m_last_quantity >= table.m_large_trade_size) {
            return Xcx2FeeTable::Type::LARGE_PASSIVE;
          } else {
            return Xcx2FeeTable::Type::PASSIVE;
          }
        } else if(report.m_liquidity_flag[0] == 'A' ||
            report.m_liquidity_flag[0] == 'C') {
          if(report.m_last_quantity >= table.m_large_trade_size) {
            return Xcx2FeeTable::Type::LARGE_ACTIVE;
          } else {
            return Xcx2FeeTable::Type::ACTIVE;
          }
        } else if(report.m_liquidity_flag[0] == 'a' ||
            report.m_liquidity_flag[0] == 'd') {
          return Xcx2FeeTable::Type::HIDDEN_PASSIVE;
        } else if(report.m_liquidity_flag[0] == 'r' ||
            report.m_liquidity_flag[0] == 'D') {
          return Xcx2FeeTable::Type::HIDDEN_ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [XCX2]: \"" <<
            report.m_liquidity_flag << "\"\n";
          return Xcx2FeeTable::Type::PASSIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [XCX2]: \"" <<
          report.m_liquidity_flag << "\"\n";
        return Xcx2FeeTable::Type::PASSIVE;
      }
    }();
    auto fee = lookup_fee(table, fields, type, price_class);
    return report.m_last_quantity * fee;
  }
}

#endif
