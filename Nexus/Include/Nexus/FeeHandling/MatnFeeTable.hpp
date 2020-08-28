#ifndef NEXUS_MATN_FEE_TABLE_HPP
#define NEXUS_MATN_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
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
      m_generalFeeTable;

    /** The alternative fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, CATEGORY_COUNT>
      m_alternativeFeeTable;
  };

  /**
   * Parses a MatnFeeTable from a YAML configuration.
   * @param config The configuration to parse the MatnFeeTable from.
   * @return The MatnFeeTable represented by the <i>config</i>.
   */
  inline MatnFeeTable ParseMatnFeeTable(const YAML::Node& config) {
    auto feeTable = MatnFeeTable();
    ParseFeeTable(config, "general_table",
      Beam::Store(feeTable.m_generalFeeTable));
    ParseFeeTable(config, "alternative_table",
      Beam::Store(feeTable.m_alternativeFeeTable));
    return feeTable;
  }

  /**
   * Looks up a fee in the general fee table.
   * @param feeTable The MatnFeeTable used to lookup the fee.
   * @param priceClass The trade's PriceClass.
   * @param generalIndex The trade's general index.
   * @return The fee corresponding to the specified <i>priceClass</i> and
   *         <i>generalIndex</i>.
   */
  inline Money LookupFee(const MatnFeeTable& feeTable,
      MatnFeeTable::GeneralIndex generalIndex,
      MatnFeeTable::PriceClass priceClass) {
    return feeTable.m_generalFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(generalIndex)];
  }

  /**
   * Looks up a fee in the alternative fee table.
   * @param feeTable The MatnFeeTable used to lookup the fee.
   * @param liquidityFlag The trade's LiquidityFlag.
   * @param category The trade's Category.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>category</i>.
   */
  inline Money LookupFee(const MatnFeeTable& feeTable,
      LiquidityFlag liquidityFlag, MatnFeeTable::Category category) {
    return feeTable.m_alternativeFeeTable[static_cast<int>(category)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Calculates the fee on a trade executed on MATN.
   * @param feeTable The MatnFeeTable used to calculate the fee.
   * @param classification The Security's classification.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const MatnFeeTable& feeTable,
      MatnFeeTable::Classification classification,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    if(classification == MatnFeeTable::Classification::DEFAULT &&
        executionReport.m_lastQuantity >= 100) {
      auto priceClass = [&] {
        if(executionReport.m_lastPrice < Money::ONE) {
          return MatnFeeTable::PriceClass::SUBDOLLAR;
        } else if(executionReport.m_lastPrice < 5 * Money::ONE) {
          return MatnFeeTable::PriceClass::SUBFIVE_DOLLAR;
        } else {
          return MatnFeeTable::PriceClass::DEFAULT;
        }
      }();
      auto fee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
        priceClass);
      auto maxCharge = LookupFee(feeTable,
        MatnFeeTable::GeneralIndex::MAX_CHARGE, priceClass);
      return std::min(executionReport.m_lastQuantity * fee, maxCharge);
    } else {
      auto liquidityFlag = [&] {
        if(executionReport.m_liquidityFlag.size() == 1) {
          if(executionReport.m_liquidityFlag[0] == 'P') {
            return LiquidityFlag::PASSIVE;
          } else if(executionReport.m_liquidityFlag[0] == 'A') {
            return LiquidityFlag::ACTIVE;
          } else {
            std::cout << "Unknown liquidity flag [MATN]: \"" <<
              executionReport.m_liquidityFlag << "\"\n";
            return LiquidityFlag::ACTIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [MATN]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return LiquidityFlag::ACTIVE;
        }
      }();
      auto category = [&] {
        if(executionReport.m_lastQuantity < 100) {
          return MatnFeeTable::Category::ODD_LOT;
        } else if(classification == MatnFeeTable::Classification::ETF) {
          return MatnFeeTable::Category::ETF;
        } else {
          std::cout << "Unknown trade category [MATN].\n";
          return MatnFeeTable::Category::ETF;
        }
      }();
      auto fee = LookupFee(feeTable, liquidityFlag, category);
      return executionReport.m_lastQuantity * fee;
    }
  }
}

#endif
