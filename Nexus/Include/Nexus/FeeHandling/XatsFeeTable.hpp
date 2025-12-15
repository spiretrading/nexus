#ifndef NEXUS_XATS_FEE_TABLE_HPP
#define NEXUS_XATS_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by Alpha on TSX. */
  struct XatsFeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price < $0.50. */
      SUBHALF_DOLLAR = 0,

      /** Price >= $0.50 & < $1.00. */
      SUBDOLLAR,

      /** Price >= $1.00 & < $5.00. */
      SUBFIVE_DOLLAR,

      /** Price >= $5.00. */
      DEFAULT,
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(4);

    /** Enumerates the types of trades. */
    enum class Type {

      /** Unknown. */
      NONE = -1,

      /** Continuous - Active, including odd-lots. */
      ACTIVE = 0,

      /** Continuous - Passive. */
      PASSIVE,

      /** Auction Opening */
      OPEN_AUCTION,

      /** IntraSpread SDL-to-Dark Active */
      SDL_ACTIVE,

      /** IntraSpread SDL-to-Dark Passive */
      SDL_PASSIVE,

      /** IntraSpread Dark-to-Active */
      INTRASPREAD_ACTIVE,

      /** IntraSpread Dark-to-Passive */
      INTRASPREAD_PASSIVE
    };

    /** The number of trade types enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(7);

    /** The general fee table. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT>
      m_general_fee_table;

    /** The ETF fee table. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT>
      m_etf_fee_table;

    /**
     * The max fee charged on IntraSpread Dark executions that are below a
     * dollar.
     */
    Money m_intraspread_dark_to_dark_subdollar_max_fee;

    /** The max fee charged on IntraSpread Dark executions. */
    Money m_intraspread_dark_to_dark_max_fee;
  };

  /**
   * Parses an XatsFeeTable from a YAML configuration.
   * @param config The configuration to parse the XatsFeeTable from.
   * @return The XatsFeeTable represented by the <i>config</i>.
   */
  inline XatsFeeTable parse_xats_fee_table(const YAML::Node& config) {
    auto table = XatsFeeTable();
    parse_fee_table(
      config, "general_table", Beam::out(table.m_general_fee_table));
    parse_fee_table(config, "etf_table", Beam::out(table.m_etf_fee_table));
    table.m_intraspread_dark_to_dark_subdollar_max_fee =
      Beam::extract<Money>(config, "intraspread_dark_to_dark_subdollar_max");
    table.m_intraspread_dark_to_dark_max_fee =
      Beam::extract<Money>(config, "intraspread_dark_to_dark_max");
    return table;
  }

  /**
   * Looks up a general fee.
   * @param table The XatsFeeTable used to lookup the fee.
   * @param type The trade's Type.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>type</i> and
   *         <i>price_class</i>.
   */
  inline Money lookup_general_fee(const XatsFeeTable& table,
      XatsFeeTable::Type type, XatsFeeTable::PriceClass price_class) {
    return table.m_general_fee_table[static_cast<int>(price_class)][
      static_cast<int>(type)];
  }

  /**
   * Looks up an ETF fee.
   * @param table The XatsFeeTable used to lookup the fee.
   * @param type The trade's Type.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>type</i> and
   *         <i>price_class</i>.
   */
  inline Money lookup_etf_fee(const XatsFeeTable& table,
      XatsFeeTable::Type type, XatsFeeTable::PriceClass price_class) {
    return table.m_etf_fee_table[static_cast<int>(price_class)][
      static_cast<int>(type)];
  }

  /**
   * Calculates the fee on a trade executed on XATS.
   * @param table The XatsFeeTable used to calculate the fee.
   * @param is_etf Whether the calculation is for an ETF.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(
      const XatsFeeTable& table, bool is_etf, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto price_class = [&] {
      if(report.m_last_price < 50 * Money::CENT) {
        return XatsFeeTable::PriceClass::SUBHALF_DOLLAR;
      } else if(report.m_last_price < Money::ONE) {
        return XatsFeeTable::PriceClass::SUBDOLLAR;
      } else if(report.m_last_price < 5 * Money::ONE) {
        return XatsFeeTable::PriceClass::SUBFIVE_DOLLAR;
      } else {
        return XatsFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto type = [&] {
      if(report.m_liquidity_flag.size() == 1) {
        if(report.m_liquidity_flag[0] == 'P') {
          return XatsFeeTable::Type::PASSIVE;
        } else if(report.m_liquidity_flag[0] == 'A') {
          return XatsFeeTable::Type::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [XATS]: " <<
            report.m_liquidity_flag << "\n";
          return XatsFeeTable::Type::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [XATS]: " <<
          report.m_liquidity_flag << "\n";
        return XatsFeeTable::Type::ACTIVE;
      }
    }();
    auto fee = [&] {
      if(is_etf) {
        return lookup_etf_fee(table, type, price_class);
      } else {
        return lookup_general_fee(table, type, price_class);
      }
    }();
    return report.m_last_quantity * fee;
  }
}

#endif
