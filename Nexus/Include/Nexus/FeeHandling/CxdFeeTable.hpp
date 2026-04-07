#ifndef NEXUS_CXD_FEE_TABLE_HPP
#define NEXUS_CXD_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by CXD. */
  struct CxdFeeTable {

    /** Enumerates the types of security classifications. */
    enum class SecurityClass {

      /** Default class. */
      DEFAULT = 0,

      /** ETF listing. */
      ETF
    };

    /** The number of security classes enumerated. */
    static constexpr auto SECURITY_CLASS_COUNT = std::size_t(2);

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Price < $1.00 */
      SUBDOLLAR = 0,

      /** $1.00 <= Price < $5.00 */
      SUBFIVE,

      /** Price >= $5.00 */
      DEFAULT
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(3);

    /** Enumerates the BBO location. */
    enum class BboType {

      /** Unknown. */
      NONE = -1,

      /** Trade happened at the BBO. */
      AT_BBO = 0,

      /** Trade happened inside of the BBO. */
      INSIDE_BBO
    };

    /** The number of BBO types enumerated. */
    static constexpr auto BBO_TYPE_COUNT = std::size_t(2);

    /** The number of column pairs (BboClass, PriceClass). */
    static constexpr auto BBO_PRICE_PAIR_COUNT =
      BBO_TYPE_COUNT * PRICE_CLASS_COUNT;

    /** The fee table for default securities. */
    std::array<std::array<Money, BBO_PRICE_PAIR_COUNT>, LIQUIDITY_FLAG_COUNT>
      m_fee_table;

    /** The max fee table for default securities. */
    std::array<std::array<Money, BBO_PRICE_PAIR_COUNT>, LIQUIDITY_FLAG_COUNT>
      m_max_fee_table;

    /** The fee table for ETF securities. */
    std::array<std::array<Money, BBO_PRICE_PAIR_COUNT>, LIQUIDITY_FLAG_COUNT>
      m_etf_fee_table;
  };

  /**
   * Parses a CxdFeeTable from a YAML configuration.
   * @param config The configuration to parse the CxdFeeTable from.
   * @return The CxdFeeTable represented by the <i>config</i>.
   */
  inline CxdFeeTable parse_cxd_fee_table(const YAML::Node& config) {
    auto table = CxdFeeTable();
    parse_fee_table(config, "fee_table", Beam::out(table.m_fee_table));
    parse_fee_table(config, "max_fee_table", Beam::out(table.m_max_fee_table));
    parse_fee_table(config, "etf_fee_table", Beam::out(table.m_etf_fee_table));
    return table;
  }

  /**
   * Returns the BboType for a given liquidity flag character.
   * @param flag The liquidity flag field.
   * @return The corresponding BboType.
   */
  inline CxdFeeTable::BboType get_cxd_bbo_type(const std::string& flag) {
    if(flag == "a" || flag == "r") {
      return CxdFeeTable::BboType::INSIDE_BBO;
    } else if(flag == "d" || flag == "D") {
      return CxdFeeTable::BboType::AT_BBO;
    }
    return CxdFeeTable::BboType::NONE;
  }

  /**
   * Returns the LiquidityFlag for a given liquidity flag character.
   * @param flag The liquidity flag character.
   * @return The corresponding LiquidityFlag.
   */
  inline LiquidityFlag get_cxd_liquidity_flag(const std::string& flag) {
    if(flag == "a" || flag == "d") {
      return LiquidityFlag::PASSIVE;
    } else if(flag == "r" || flag == "D") {
      return LiquidityFlag::ACTIVE;
    }
    return LiquidityFlag::NONE;
  }

  /**
   * Returns the PriceClass for a given trade.
   * @param price The price of the trade to classify.
   * @return The corresponding PriceClass.
   */
  inline CxdFeeTable::PriceClass get_cxd_price_class(Money price) {
    if(price < Money::ONE) {
      return CxdFeeTable::PriceClass::SUBDOLLAR;
    } else if(price < 5 * Money::ONE) {
      return CxdFeeTable::PriceClass::SUBFIVE;
    }
    return CxdFeeTable::PriceClass::DEFAULT;
  }

  /**
   * Looks up the fee in the CXD fee table.
   * @param table The CxdFeeTable to use.
   * @param flag The LiquidityFlag (ACTIVE or PASSIVE).
   * @param security_class The SecurityClass (DEFAULT or ETF).
   * @param price_class The PriceClass (SUBDOLLAR, SUBFIVE, DEFAULT).
   * @param bbo_type The BboType (AT_BBO or INSIDE_BBO).
   * @return The fee for the specified parameters.
   */
  inline Money lookup_fee(const CxdFeeTable& table, LiquidityFlag flag,
      CxdFeeTable::SecurityClass security_class,
      CxdFeeTable::PriceClass price_class, CxdFeeTable::BboType bbo_type) {
    auto bbo_index = static_cast<std::size_t>(bbo_type);
    auto price_index = static_cast<std::size_t>(price_class);
    auto liquidity_index = static_cast<std::size_t>(flag);
    auto column_index =
      bbo_index * CxdFeeTable::PRICE_CLASS_COUNT + price_index;
    if(security_class == CxdFeeTable::SecurityClass::ETF) {
      return table.m_etf_fee_table[liquidity_index][column_index];
    }
    return table.m_fee_table[liquidity_index][column_index];
  }

  /**
   * Looks up the max fee in the CXD fee table.
   * @param table The CxdFeeTable to use.
   * @param flag The LiquidityFlag (ACTIVE or PASSIVE).
   * @param price_class The PriceClass (SUBDOLLAR, SUBFIVE, DEFAULT).
   * @param bbo_type The BboType (AT_BBO or INSIDE_BBO).
   * @return The max fee for the specified parameters.
   */
  inline Money lookup_max_fee(const CxdFeeTable& table, LiquidityFlag flag,
      CxdFeeTable::PriceClass price_class, CxdFeeTable::BboType bbo_type) {
    auto bbo_index = static_cast<std::size_t>(bbo_type);
    auto price_index = static_cast<std::size_t>(price_class);
    auto liquidity_index = static_cast<std::size_t>(flag);
    auto column_index =
      bbo_index * CxdFeeTable::PRICE_CLASS_COUNT + price_index;
    return table.m_max_fee_table[liquidity_index][column_index];
  }

  /**
   * Calculates the fee for a trade executed on CXD.
   * @param table The CxdFeeTable to use.
   * @param security_class The SecurityClass (DEFAULT or ETF).
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee for the specified trade.
   */
  inline Money calculate_fee(
      const CxdFeeTable& table, CxdFeeTable::SecurityClass security_class,
      const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto bbo_type = get_cxd_bbo_type(report.m_liquidity_flag);
    auto flag = get_cxd_liquidity_flag(report.m_liquidity_flag);
    if(bbo_type == CxdFeeTable::BboType::NONE || flag == LiquidityFlag::NONE) {
      std::cout << "Unknown liquidity flag [CXD]: \"" <<
        report.m_liquidity_flag << "\"\n";
      bbo_type = CxdFeeTable::BboType::AT_BBO;
      flag = LiquidityFlag::ACTIVE;
    }
    auto price_class = get_cxd_price_class(report.m_last_price);
    auto per_share_fee =
      lookup_fee(table, flag, security_class, price_class, bbo_type);
    if(security_class == CxdFeeTable::SecurityClass::ETF) {
      return report.m_last_quantity * per_share_fee;
    } else {
      auto max_fee = lookup_max_fee(table, flag, price_class, bbo_type);
      return std::min(report.m_last_quantity * per_share_fee, max_fee);
    }
  }
}

#endif
