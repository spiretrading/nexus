#ifndef NEXUS_NEOE_FEE_TABLE_HPP
#define NEXUS_NEOE_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by Aequitas NEO on TSX. */
  struct NeoeFeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price < $1.00. */
      SUBDOLLAR,

      /** Price >= $1.00. */
      DEFAULT
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(2);

    /** Enumerates the listing classifications. */
    enum class Classification {

      /** General listing. */
      GENERAL,

      /** Interlisted security. */
      INTERLISTED,

      /** ETF listing. */
      ETF
    };

    /** The number of listing classifications. */
    static constexpr auto CLASSIFICATION_COUNT = std::size_t(3);

    /** The general fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_general_fee_table;

    /** The interlisted fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_interlisted_fee_table;

    /** The ETF fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_etf_table_fee;

    /** The NEO book fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_neo_book_fee_table;
  };

  /**
   * Parses a NeoeFeeTable from a YAML configuration.
   * @param config The configuration to parse the NeoeFeeTable from.
   * @return The NeoeFeeTable represented by the <i>config</i>.
   */
  inline NeoeFeeTable parse_neoe_fee_table(const YAML::Node& config) {
    auto table = NeoeFeeTable();
    parse_fee_table(
      config, "general_table", Beam::out(table.m_general_fee_table));
    parse_fee_table(
      config, "interlisted_table", Beam::out(table.m_interlisted_fee_table));
    parse_fee_table(
      config, "etf_table", Beam::out(table.m_interlisted_fee_table));
    parse_fee_table(
      config, "neo_book_table", Beam::out(table.m_neo_book_fee_table));
    return table;
  }

  /**
   * Tests whether a NEO Order is part of the NEO book.
   * @param fields The OrderFields to test.
   * @return <code>true</code> iff the <i>order</i> was submitted to the NEO
   *         book.
   */
  inline bool is_neo_book_order(const OrderFields& fields) {
    return has_field(fields, Tag(100, "N"));
  }

  /**
   * Looks up a general fee.
   * @param table The NeoeFeeTable used to lookup the fee.
   * @param flag The trade's LiquidityFlag.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>flag</i> and
   *         <i>price_class</i>.
   */
  inline Money lookup_general_fee(const NeoeFeeTable& table,
      LiquidityFlag flag, NeoeFeeTable::PriceClass price_class) {
    return table.m_general_fee_table[static_cast<int>(price_class)][
      static_cast<int>(flag)];
  }

  /**
   * Looks up an interlisted fee.
   * @param table The NeoeFeeTable used to lookup the fee.
   * @param flag The trade's LiquidityFlag.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>flag</i> and
   *         <i>price_class</i>.
   */
  inline Money lookup_interlisted_fee(const NeoeFeeTable& table,
      LiquidityFlag flag, NeoeFeeTable::PriceClass price_class) {
    return table.m_interlisted_fee_table[static_cast<int>(price_class)][
      static_cast<int>(flag)];
  }

  /**
   * Looks up an ETF fee.
   * @param table The NeoeFeeTable used to lookup the fee.
   * @param flag The trade's LiquidityFlag.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>flag</i> and
   *         <i>price_class</i>.
   */
  inline Money lookup_etf_fee(const NeoeFeeTable& table, LiquidityFlag flag,
      NeoeFeeTable::PriceClass price_class) {
    return table.m_etf_table_fee[static_cast<int>(price_class)][
      static_cast<int>(flag)];
  }

  /**
   * Looks up a NEO book fee.
   * @param table The NeoeFeeTable used to lookup the fee.
   * @param flag The trade's LiquidityFlag.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>flag</i>.
   */
  inline Money lookup_neo_book_fee(const NeoeFeeTable& table,
      LiquidityFlag flag, NeoeFeeTable::PriceClass price_class) {
    return table.m_neo_book_fee_table[static_cast<int>(price_class)][
      static_cast<int>(flag)];
  }

  /**
   * Calculates the fee on a trade executed on NEOE.
   * @param table The NeoeFeeTable used to calculate the fee.
   * @param classification The listing classification of the security.
   * @param fields The OrderFields submitted for the Order.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(
      const NeoeFeeTable& table, NeoeFeeTable::Classification classification,
      const OrderFields& fields, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto price_class = [&] {
      if(report.m_last_price < Money::ONE) {
        return NeoeFeeTable::PriceClass::SUBDOLLAR;
      } else {
        return NeoeFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto flag = [&] {
      if(report.m_liquidity_flag.size() == 1) {
        if(report.m_liquidity_flag[0] == 'P') {
          return LiquidityFlag::PASSIVE;
        } else if(report.m_liquidity_flag[0] == 'A') {
          return LiquidityFlag::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [NEOE]: " <<
            report.m_liquidity_flag << "\n";
          return LiquidityFlag::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [NEOE]: " <<
          report.m_liquidity_flag << "\n";
        return LiquidityFlag::ACTIVE;
      }
    }();
    auto fee = [&] {
      if(is_neo_book_order(fields)) {
        return lookup_neo_book_fee(table, flag, price_class);
      } else if(classification == NeoeFeeTable::Classification::INTERLISTED) {
        return lookup_interlisted_fee(table, flag, price_class);
      } else if(classification == NeoeFeeTable::Classification::ETF) {
        return lookup_etf_fee(table, flag, price_class);
      } else {
        return lookup_general_fee(table, flag, price_class);
      }
    }();
    return report.m_last_quantity * fee;
  }
}

#endif
