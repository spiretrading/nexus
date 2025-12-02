#ifndef NEXUS_ASXT_FEE_TABLE_HPP
#define NEXUS_ASXT_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by ASX TradeMatch. */
  struct AsxTradeMatchFeeTable {

    /** Used to categorize fees based on the price of the security. */
    enum class PriceClass {

      /** Not recognized. */
      NONE = -1,

      /** Price < 0.25 */
      TIER_ONE,

      /** 0.25 <= Price < 1.00 */
      TIER_TWO,

      /** Price >= 1.00 */
      TIER_THREE
    };

    /** The number of price classes. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(3);

    /** Used to categorize fees based on the order type. */
    enum class OrderTypeClass {

      /** Not recognized. */
      NONE = -1,

      /** Non-pegged order. */
      REGULAR,

      /** Pegged order. */
      PEGGED
    };

    /** The number of order type classes. */
    static constexpr auto ORDER_TYPE_COUNT = std::size_t(2);

    /** Fee charged for the software. */
    Money m_spire_fee;

    /** The clearing rates. */
    std::array<std::array<boost::rational<int>, PRICE_CLASS_COUNT>,
      ORDER_TYPE_COUNT> m_clearing_rate_table;

    /** The equities trade rate. */
    boost::rational<int> m_trade_rate;

    /** The GST rate. */
    boost::rational<int> m_gst_rate;

    /** The cap on the trade fee. */
    Money m_trade_fee_cap;
  };

  /**
   * Parses an AsxTradeMatchFeeTable from a YAML configuration.
   * @param config The configuration to parse the AsxTradeMatchFeeTable from.
   * @return The AsxTradeMatchFeeTable represented by the <i>config</i>.
   */
  inline AsxTradeMatchFeeTable
      parse_asx_trade_match_fee_table(const YAML::Node& config) {
    auto table = AsxTradeMatchFeeTable();
    table.m_spire_fee = Beam::extract<Money>(config, "spire_fee");
    parse_fee_table(
      config, "clearing_rate_table", Beam::out(table.m_clearing_rate_table));
    table.m_trade_rate =
      Beam::extract<boost::rational<int>>(config, "trade_rate");
    table.m_gst_rate = Beam::extract<boost::rational<int>>(config, "gst_rate");
    table.m_trade_fee_cap = Beam::extract<Money>(config, "trade_fee_cap");
    return table;
  }

  /**
   * Looks up a clearing fee.
   * @param table The AsxTradeMatchFeeTable used to lookup the fee.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>priceClass</i>.
   */
  inline boost::rational<int> lookup_clearing_fee(
      const AsxTradeMatchFeeTable& table,
      AsxTradeMatchFeeTable::PriceClass price_class,
      AsxTradeMatchFeeTable::OrderTypeClass order_type_class) {
    return table.m_clearing_rate_table[static_cast<int>(order_type_class)][
      static_cast<int>(price_class)];
  }

  /**
   * Looks up a trade's price class.
   * @param report The execution report representing the trade.
   * @return The trade's price class.
   */
  inline AsxTradeMatchFeeTable::PriceClass lookup_price_class(
      const ExecutionReport& report) {
    if(report.m_last_price < 25 * Money::CENT) {
      return AsxTradeMatchFeeTable::PriceClass::TIER_ONE;
    } else if(report.m_last_price < Money::ONE) {
      return AsxTradeMatchFeeTable::PriceClass::TIER_TWO;
    }
    return AsxTradeMatchFeeTable::PriceClass::TIER_THREE;
  }

  /**
   * Looks up an order type class.
   * @param fields The fields that were submitted.
   * @return The order's type class.
   */
  inline AsxTradeMatchFeeTable::OrderTypeClass lookup_order_type_class(
      const OrderFields& fields) {
    if(fields.m_type == OrderType::PEGGED) {
      return AsxTradeMatchFeeTable::OrderTypeClass::PEGGED;
    }
    return AsxTradeMatchFeeTable::OrderTypeClass::REGULAR;
  }

  /**
   * Calculates the clearing fee on a trade executed on the ASX market.
   * @param table The AsxTradeMatchFeeTable used to calculate the fee.
   * @param fields The fields that were submitted.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The execution fee for the given trade.
   */
  inline Money calculate_clearing_fee(const AsxTradeMatchFeeTable& table,
      const OrderFields& fields, const ExecutionReport& report) {
    auto price_class = lookup_price_class(report);
    auto order_type_class = lookup_order_type_class(fields);
    auto clearing_fee =
      lookup_clearing_fee(table, price_class, order_type_class);
    return (clearing_fee * report.m_last_quantity) * report.m_last_price;
  }

  /**
   * Calculates the execution fee on a trade executed on the ASX market.
   * @param table The AsxTradeMatchFeeTable used to calculate the fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The execution fee for the given trade.
   */
  inline Money calculate_execution_fee(
      const AsxTradeMatchFeeTable& table, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto notional_value = report.m_last_quantity * report.m_last_price;
    auto base_trade_fee = [&] {
      if(table.m_trade_fee_cap == Money::ZERO) {
        return table.m_trade_rate * notional_value;
      }
      return std::min(
        table.m_trade_rate * notional_value, table.m_trade_fee_cap);
    }();
    auto trade_fee = (1 + table.m_gst_rate) * base_trade_fee;
    return trade_fee;
  }

  /**
   * Calculates the fee on a trade executed on ASXT.
   * @param table The AsxTradeMatchFeeTable used to calculate the fee.
   * @param fields The fields that were submitted.
   * @param report The ExecutionReport to calculate the fee for.
   * @return An ExecutionReport containing the calculated fees.
   */
  inline ExecutionReport calculate_fee(const AsxTradeMatchFeeTable& table,
      const OrderFields& fields, const ExecutionReport& report) {
    auto fees_report = report;
    fees_report.m_processing_fee +=
      calculate_clearing_fee(table, fields, report);
    fees_report.m_execution_fee += calculate_execution_fee(table, report);
    if(fees_report.m_last_quantity != 0) {
      fees_report.m_commission += table.m_spire_fee;
    }
    return fees_report;
  }
}

#endif
