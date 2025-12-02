#ifndef NEXUS_MATN_FEE_TABLE_HPP
#define NEXUS_MATN_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by Match NOW. */
  struct MatnFeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price >= $1.00. */
      DEFAULT = 0,

      /** Price >= $1.00 & < $5.00. */
      SUBFIVE_DOLLAR,

      /** Price < $1.00 */
      SUBDOLLAR
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(3);

    /** Enumerates the indices used in the general fee table. */
    enum class GeneralIndex {

      /** Unknown. */
      NONE = -1,

      /** The fee. */
      FEE = 0,

      /** The max price charged per trade. */
      MAX_CHARGE
    };

    /** The number of general indices enumerated. */
    static constexpr auto GENERAL_INDEX_COUNT = std::size_t(2);

    /** Enumerates the categories for alternative trades. */
    enum class Category {

      /** Unknown. */
      NONE = -1,

      /** Trade on an ETF. */
      ETF = 0,

      /** Odd lot. */
      ODD_LOT
    };

    /** The number of Categories enumerated. */
    static constexpr auto CATEGORY_COUNT = std::size_t(2);

    /** Enumerates various classifications for a Security. */
    enum class Classification {

      /** Unknown. */
      NONE = -1,

      /** The default classification. */
      DEFAULT = 0,

      /** An ETF. */
      ETF
    };

    /** The number of Classifications enumerated. */
    static constexpr auto CLASSIFICATION_COUNT = std::size_t(2);

    /** The general fee table. */
    std::array<std::array<Money, GENERAL_INDEX_COUNT>, PRICE_CLASS_COUNT>
      m_general_fee_table;

    /** The alternative fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, CATEGORY_COUNT>
      m_alternative_fee_table;
  };

  /**
   * Parses a MatnFeeTable from a YAML configuration.
   * @param config The configuration to parse the MatnFeeTable from.
   * @return The MatnFeeTable represented by the <i>config</i>.
   */
  inline MatnFeeTable parse_matn_fee_table(const YAML::Node& config) {
    auto table = MatnFeeTable();
    parse_fee_table(
      config, "general_table", Beam::out(table.m_general_fee_table));
    parse_fee_table(
      config, "alternative_table", Beam::out(table.m_alternative_fee_table));
    return table;
  }

  /**
   * Looks up a fee in the general fee table.
   * @param table The MatnFeeTable used to lookup the fee.
   * @param price_class The trade's PriceClass.
   * @param general_index The trade's general index.
   * @return The fee corresponding to the specified <i>price_class</i> and
   *         <i>general_index</i>.
   */
  inline Money lookup_fee(
      const MatnFeeTable& table, MatnFeeTable::GeneralIndex general_index,
      MatnFeeTable::PriceClass price_class) {
    return table.m_general_fee_table[static_cast<int>(price_class)][
      static_cast<int>(general_index)];
  }

  /**
   * Looks up a fee in the alternative fee table.
   * @param table The MatnFeeTable used to lookup the fee.
   * @param flag The trade's LiquidityFlag.
   * @param category The trade's Category.
   * @return The fee corresponding to the specified <i>flag</i> and
   *         <i>category</i>.
   */
  inline Money lookup_fee(const MatnFeeTable& table, LiquidityFlag flag,
      MatnFeeTable::Category category) {
    return table.m_alternative_fee_table[static_cast<int>(category)][
      static_cast<int>(flag)];
  }

  /**
   * Calculates the fee on a trade executed on MATN.
   * @param table The MatnFeeTable used to calculate the fee.
   * @param classification The Security's classification.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(
      const MatnFeeTable& table, MatnFeeTable::Classification classification,
      const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    if(classification == MatnFeeTable::Classification::DEFAULT &&
        report.m_last_quantity >= 100) {
      auto price_class = [&] {
        if(report.m_last_price < Money::ONE) {
          return MatnFeeTable::PriceClass::SUBDOLLAR;
        } else if(report.m_last_price < 5 * Money::ONE) {
          return MatnFeeTable::PriceClass::SUBFIVE_DOLLAR;
        } else {
          return MatnFeeTable::PriceClass::DEFAULT;
        }
      }();
      auto fee =
        lookup_fee(table, MatnFeeTable::GeneralIndex::FEE, price_class);
      auto max_charge =
        lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE, price_class);
      return std::min(report.m_last_quantity * fee, max_charge);
    } else {
      auto flag = [&] {
        if(report.m_liquidity_flag.size() == 1) {
          if(report.m_liquidity_flag[0] == 'P') {
            return LiquidityFlag::PASSIVE;
          } else if(report.m_liquidity_flag[0] == 'A') {
            return LiquidityFlag::ACTIVE;
          } else {
            std::cout << "Unknown liquidity flag [MATN]: \"" <<
              report.m_liquidity_flag << "\"\n";
            return LiquidityFlag::ACTIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [MATN]: \"" <<
            report.m_liquidity_flag << "\"\n";
          return LiquidityFlag::ACTIVE;
        }
      }();
      auto category = [&] {
        if(report.m_last_quantity < 100) {
          return MatnFeeTable::Category::ODD_LOT;
        } else if(classification == MatnFeeTable::Classification::ETF) {
          return MatnFeeTable::Category::ETF;
        } else {
          std::cout << "Unknown trade category [MATN].\n";
          return MatnFeeTable::Category::ETF;
        }
      }();
      auto fee = lookup_fee(table, flag, category);
      return report.m_last_quantity * fee;
    }
  }
}

#endif
