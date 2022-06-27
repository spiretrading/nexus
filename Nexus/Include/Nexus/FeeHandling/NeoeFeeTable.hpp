#ifndef NEXUS_NEOE_FEE_TABLE_HPP
#define NEXUS_NEOE_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
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
      m_generalFeeTable;

    /** The interlisted fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_interlistedFeeTable;

    /** The ETF fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_etfFeeTable;

    /** The NEO book fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_neoBookFeeTable;
  };

  /**
   * Parses a NeoeFeeTable from a YAML configuration.
   * @param config The configuration to parse the NeoeFeeTable from.
   * @return The NeoeFeeTable represented by the <i>config</i>.
   */
  inline NeoeFeeTable ParseNeoeFeeTable(const YAML::Node& config) {
    auto feeTable = NeoeFeeTable();
    ParseFeeTable(
      config, "general_table", Beam::Store(feeTable.m_generalFeeTable));
    ParseFeeTable(
      config, "interlisted_table", Beam::Store(feeTable.m_interlistedFeeTable));
    ParseFeeTable(
      config, "etf_table", Beam::Store(feeTable.m_interlistedFeeTable));
    ParseFeeTable(
      config, "neo_book_table", Beam::Store(feeTable.m_neoBookFeeTable));
    return feeTable;
  }

  /**
   * Tests whether a NEO Order is part of the NEO book.
   * @param fields The OrderFields to test.
   * @return <code>true</code> iff the <i>order</i> was submitted to the NEO
   *         book.
   */
  inline bool IsNeoBookOrder(const OrderExecutionService::OrderFields& fields) {
    return OrderExecutionService::HasField(fields, Tag(100, "N"));
  }

  /**
   * Looks up a general fee.
   * @param feeTable The NeoeFeeTable used to lookup the fee.
   * @param liquidityFlag The trade's LiquidityFlag.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupGeneralFee(const NeoeFeeTable& feeTable,
      LiquidityFlag liquidityFlag, NeoeFeeTable::PriceClass priceClass) {
    return feeTable.m_generalFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Looks up an interlisted fee.
   * @param feeTable The NeoeFeeTable used to lookup the fee.
   * @param liquidityFlag The trade's LiquidityFlag.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupInterlistedFee(const NeoeFeeTable& feeTable,
      LiquidityFlag liquidityFlag, NeoeFeeTable::PriceClass priceClass) {
    return feeTable.m_interlistedFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Looks up an ETF fee.
   * @param feeTable The NeoeFeeTable used to lookup the fee.
   * @param liquidityFlag The trade's LiquidityFlag.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupEtfFee(const NeoeFeeTable& feeTable,
      LiquidityFlag liquidityFlag, NeoeFeeTable::PriceClass priceClass) {
    return feeTable.m_etfFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Looks up a NEO book fee.
   * @param feeTable The NeoeFeeTable used to lookup the fee.
   * @param liquidityFlag The trade's LiquidityFlag.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>liquidityFlag</i>.
   */
  inline Money LookupNeoBookFee(const NeoeFeeTable& feeTable,
      LiquidityFlag liquidityFlag, NeoeFeeTable::PriceClass priceClass) {
    return feeTable.m_neoBookFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Calculates the fee on a trade executed on NEOE.
   * @param feeTable The NeoeFeeTable used to calculate the fee.
   * @param classification The listing classification of the security.
   * @param orderFields The OrderFields submitted for the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const NeoeFeeTable& feeTable,
      NeoeFeeTable::Classification classification,
      const OrderExecutionService::OrderFields& orderFields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < Money::ONE) {
        return NeoeFeeTable::PriceClass::SUBDOLLAR;
      } else {
        return NeoeFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto liquidityFlag = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P') {
          return LiquidityFlag::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'A') {
          return LiquidityFlag::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [NEOE]: " <<
            executionReport.m_liquidityFlag << "\n";
          return LiquidityFlag::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [NEOE]: " <<
          executionReport.m_liquidityFlag << "\n";
        return LiquidityFlag::ACTIVE;
      }
    }();
    auto fee = [&] {
      if(IsNeoBookOrder(orderFields)) {
        return LookupNeoBookFee(feeTable, liquidityFlag, priceClass);
      } else if(classification == NeoeFeeTable::Classification::INTERLISTED) {
        return LookupInterlistedFee(feeTable, liquidityFlag, priceClass);
      } else if(classification == NeoeFeeTable::Classification::ETF) {
        return LookupEtfFee(feeTable, liquidityFlag, priceClass);
      } else {
        return LookupGeneralFee(feeTable, liquidityFlag, priceClass);
      }
    }();
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
