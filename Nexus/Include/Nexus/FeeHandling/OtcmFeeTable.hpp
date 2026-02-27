#ifndef NEXUS_OTCM_FEE_TABLE_HPP
#define NEXUS_OTCM_FEE_TABLE_HPP
#include <array>
#include <iostream>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by OTC Markets. */
  struct OtcmFeeTable {

    /** Enumerates the price classes. */
    enum class PriceClass {

      /** Price >= $1.00. */
      DEFAULT = 0,

      /** $0.01 <= Price < $1.00. */
      SUBDOLLAR,

      /** Price < $0.01. */
      SUBPENNY
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(3);

    /** Enumerates the types of trades. */
    enum class Type {

      /** Unknown. */
      NONE = -1,

      /** Adding liquidity. */
      PASSIVE = 0,

      /** Adding hidden liquidity. */
      HIDDEN_PASSIVE,

      /** Removing liquidity. */
      ACTIVE,

      /** Routed to OTC Link ATS. */
      ROUTED_TO_OTC_LINK,

      /** Routed to external ECN. */
      ROUTED_TO_EXTERNAL_ECN,

      /** Routed to external liquidity. */
      ROUTED_TO_EXTERNAL_LIQUIDITY
    };

    /** The number of trade types enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(6);

    /** The fee table for securities priced >= $1.00, indexed by Type. */
    std::array<Money, TYPE_COUNT> m_fee_table;

    /**
     * The fee table for securities with $0.01 <= price < $1.00, indexed by
     * Type.
     */
    std::array<Money, TYPE_COUNT> m_subdollar_fee_table;

    /**
     * The fee table for securities with price < $0.01, indexed by Type. Values
     * represent a percentage of notional value.
     */
    std::array<boost::rational<int>, TYPE_COUNT> m_subpenny_rate_table;

    /**
     * The closing cross fee table, indexed by [LiquidityFlag][PriceClass]. The
     * DEFAULT and SUBDOLLAR entries are per-share fees and the SUBPENNY entry
     * represents a percentage of notional value.
     */
    std::array<std::array<Money, PRICE_CLASS_COUNT>, LIQUIDITY_FLAG_COUNT>
      m_closing_cross_fee_table;

    /**
     * The closing cross subpenny rate table, indexed by LiquidityFlag. Values
     * represent a percentage of notional value.
     */
    std::array<boost::rational<int>, LIQUIDITY_FLAG_COUNT>
      m_closing_cross_subpenny_rate_table;
  };

  /**
   * Parses an OtcmFeeTable from a YAML configuration.
   * @param config The configuration to parse the OtcmFeeTable from.
   * @return The OtcmFeeTable represented by the <i>config</i>.
   */
  inline OtcmFeeTable parse_otcm_fee_table(const YAML::Node& config) {
    auto table = OtcmFeeTable();
    parse_fee_table(config, "fee_table", Beam::out(table.m_fee_table));
    parse_fee_table(
      config, "subdollar_fee_table", Beam::out(table.m_subdollar_fee_table));
    parse_fee_table(
      config, "subpenny_rate_table", Beam::out(table.m_subpenny_rate_table));
    parse_fee_table(config, "closing_cross_fee_table",
      Beam::out(table.m_closing_cross_fee_table));
    parse_fee_table(config, "closing_cross_subpenny_rate_table",
      Beam::out(table.m_closing_cross_subpenny_rate_table));
    return table;
  }

  /**
   * Looks up the per-share fee for securities priced >= $1.00.
   * @param table The OtcmFeeTable used to lookup the fee.
   * @param type The trade's Type.
   * @return The per-share fee for the specified <i>type</i>.
   */
  inline Money lookup_fee(const OtcmFeeTable& table, OtcmFeeTable::Type type) {
    return table.m_fee_table[static_cast<int>(type)];
  }

  /**
   * Looks up the per-share fee for securities with $0.01 <= price < $1.00.
   * @param table The OtcmFeeTable used to lookup the fee.
   * @param type The trade's Type.
   * @return The per-share fee for the specified <i>type</i>.
   */
  inline Money lookup_subdollar_fee(
      const OtcmFeeTable& table, OtcmFeeTable::Type type) {
    return table.m_subdollar_fee_table[static_cast<int>(type)];
  }

  /**
   * Looks up the notional rate for securities with price < $0.01.
   * @param table The OtcmFeeTable used to lookup the rate.
   * @param type The trade's Type.
   * @return The notional rate for the specified <i>type</i>.
   */
  inline boost::rational<int> lookup_subpenny_rate(
      const OtcmFeeTable& table, OtcmFeeTable::Type type) {
    return table.m_subpenny_rate_table[static_cast<int>(type)];
  }

  /**
   * Returns the PriceClass for a given price.
   * @param price The trade price to classify.
   * @return The PriceClass for the specified <i>price</i>.
   */
  inline OtcmFeeTable::PriceClass get_otcm_price_class(Money price) {
    if(price < Money::CENT) {
      return OtcmFeeTable::PriceClass::SUBPENNY;
    } else if(price < Money::ONE) {
      return OtcmFeeTable::PriceClass::SUBDOLLAR;
    } else {
      return OtcmFeeTable::PriceClass::DEFAULT;
    }
  }

  /**
   * Looks up the closing cross fee for a given price class and liquidity flag.
   * @param table The OtcmFeeTable used to lookup the fee.
   * @param flag The LiquidityFlag.
   * @param price_class The trade's PriceClass.
   * @return The fee for the specified <i>flag</i> and <i>price_class</i>.
   */
  inline Money lookup_closing_cross_fee(const OtcmFeeTable& table,
      LiquidityFlag flag, OtcmFeeTable::PriceClass price_class) {
    return table.m_closing_cross_fee_table[static_cast<int>(flag)][
      static_cast<int>(price_class)];
  }

  /**
   * Looks up the closing cross subpenny notional rate for a given liquidity
   * flag.
   * @param table The OtcmFeeTable used to lookup the rate.
   * @param flag The LiquidityFlag.
   * @return The notional rate for the specified <i>flag</i>.
   */
  inline boost::rational<int> lookup_closing_cross_subpenny_rate(
      const OtcmFeeTable& table, LiquidityFlag flag) {
    return table.m_closing_cross_subpenny_rate_table[static_cast<int>(flag)];
  }

  /**
   * Returns the Type for a given liquidity flag on a non-closing-cross trade.
   * @param flag The liquidity flag field.
   * @return The Type for the specified <i>flag</i>, or NONE if the flag is
   *         not recognized.
   */
  inline OtcmFeeTable::Type get_otcm_type(const std::string& flag) {
    if(flag.size() == 1) {
      if(flag[0] == 'A' || flag[0] == 'D') {
        return OtcmFeeTable::Type::PASSIVE;
      } else if(flag[0] == 'B' || flag[0] == 'E') {
        return OtcmFeeTable::Type::HIDDEN_PASSIVE;
      } else if(flag[0] == 'R' || flag[0] == 'H') {
        return OtcmFeeTable::Type::ACTIVE;
      } else if(flag[0] == 'L') {
        return OtcmFeeTable::Type::ROUTED_TO_OTC_LINK;
      } else if(flag[0] == 'X') {
        return OtcmFeeTable::Type::ROUTED_TO_EXTERNAL_ECN;
      } else if(flag[0] == 'P') {
        return OtcmFeeTable::Type::ROUTED_TO_EXTERNAL_LIQUIDITY;
      }
    }
    return OtcmFeeTable::Type::NONE;
  }

  /**
   * Returns the LiquidityFlag for a closing cross liquidity flag.
   * @param flag The liquidity flag field.
   * @return The LiquidityFlag for the specified <i>flag</i>, or NONE if the
   *         flag is not recognized.
   */
  inline LiquidityFlag get_otcm_closing_cross_type(
      const std::string& flag) {
    if(flag.size() == 1) {
      if(flag[0] == 'a' || flag[0] == 'd') {
        return LiquidityFlag::PASSIVE;
      } else if(flag[0] == 'r' || flag[0] == 'h') {
        return LiquidityFlag::ACTIVE;
      }
    }
    return LiquidityFlag::NONE;
  }

  /**
   * Calculates the fee on a trade executed on OTCM.
   * @param table The OtcmFeeTable used to calculate the fee.
   * @param fields The OrderFields the trade took place on.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(const OtcmFeeTable& table,
      const OrderFields& fields, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto price_class = get_otcm_price_class(report.m_last_price);
    auto closing_cross_type =
      get_otcm_closing_cross_type(report.m_liquidity_flag);
    if(closing_cross_type != LiquidityFlag::NONE) {
      if(price_class == OtcmFeeTable::PriceClass::SUBPENNY) {
        auto notional = report.m_last_quantity * report.m_last_price;
        return lookup_closing_cross_subpenny_rate(
          table, closing_cross_type) * notional;
      }
      return report.m_last_quantity *
        lookup_closing_cross_fee(table, closing_cross_type, price_class);
    }
    auto type = get_otcm_type(report.m_liquidity_flag);
    if(type == OtcmFeeTable::Type::NONE) {
      std::cout << "Unknown liquidity flag [OTCM]: \"" <<
        report.m_liquidity_flag << "\"\n";
      type = OtcmFeeTable::Type::ACTIVE;
    }
    if(price_class == OtcmFeeTable::PriceClass::SUBPENNY) {
      auto notional = report.m_last_quantity * report.m_last_price;
      return lookup_subpenny_rate(table, type) * notional;
    } else if(price_class == OtcmFeeTable::PriceClass::SUBDOLLAR) {
      return report.m_last_quantity * lookup_subdollar_fee(table, type);
    } else {
      return report.m_last_quantity * lookup_fee(table, type);
    }
  }
}

#endif
