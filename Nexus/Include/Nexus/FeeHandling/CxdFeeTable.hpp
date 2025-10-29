#ifndef NEXUS_CXD_FEE_TABLE_HPP
#define NEXUS_CXD_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
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
      m_feeTable;

    /** The max fee table for default securities. */
    std::array<std::array<Money, BBO_PRICE_PAIR_COUNT>, LIQUIDITY_FLAG_COUNT>
      m_maxFeeTable;

    /** The fee table for ETF securities. */
    std::array<std::array<Money, BBO_PRICE_PAIR_COUNT>, LIQUIDITY_FLAG_COUNT>
      m_etfFeeTable;
  };

  /**
   * Parses a CxdFeeTable from a YAML configuration.
   * @param config The configuration to parse the CxdFeeTable from.
   * @return The CxdFeeTable represented by the <i>config</i>.
   */
  inline CxdFeeTable ParseCxdFeeTable(const YAML::Node& config) {
    auto feeTable = CxdFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    ParseFeeTable(config, "max_fee_table", Beam::Store(feeTable.m_maxFeeTable));
    ParseFeeTable(config, "etf_fee_table", Beam::Store(feeTable.m_etfFeeTable));
    return feeTable;
  }

  /**
   * Returns the BboType for a given liquidity flag character.
   * @param liquidityFlag The liquidity flag field.
   * @return The corresponding BboType.
   */
  inline CxdFeeTable::BboType GetCxdBboType(const std::string& liquidityFlag) {
    if(liquidityFlag == "a" || liquidityFlag == "r") {
      return CxdFeeTable::BboType::INSIDE_BBO;
    } else if(liquidityFlag == "d" || liquidityFlag == "D") {
      return CxdFeeTable::BboType::AT_BBO;
    }
    return CxdFeeTable::BboType::NONE;
  }

  /**
   * Returns the LiquidityFlag for a given liquidity flag character.
   * @param liquidityFlag The liquidity flag character.
   * @return The corresponding LiquidityFlag.
   */
  inline LiquidityFlag GetCxdLiquidityFlag(const std::string& liquidityFlag) {
    if(liquidityFlag == "a" || liquidityFlag == "d") {
      return LiquidityFlag::PASSIVE;
    } else if(liquidityFlag == "r" || liquidityFlag == "D") {
      return LiquidityFlag::ACTIVE;
    }
    return LiquidityFlag::NONE;
  }

  /**
   * Returns the PriceClass for a given trade.
   * @param price The price of the trade to classify.
   * @return The corresponding PriceClass.
   */
  inline CxdFeeTable::PriceClass GetCxdPriceClass(Money price) {
    if(price < Money::ONE) {
      return CxdFeeTable::PriceClass::SUBDOLLAR;
    } else if(price < 5 * Money::ONE) {
      return CxdFeeTable::PriceClass::SUBFIVE;
    }
    return CxdFeeTable::PriceClass::DEFAULT;
  }

  /**
   * Looks up the fee in the CXD fee table.
   * @param feeTable The CxdFeeTable to use.
   * @param liquidityFlag The LiquidityFlag (ACTIVE or PASSIVE).
   * @param securityClass The SecurityClass (DEFAULT or ETF).
   * @param priceClass The PriceClass (SUBDOLLAR, SUBFIVE, DEFAULT).
   * @param bboType The BboType (AT_BBO or INSIDE_BBO).
   * @return The fee for the specified parameters.
   */
  inline Money LookupFee(const CxdFeeTable& feeTable,
      LiquidityFlag liquidityFlag, CxdFeeTable::SecurityClass securityClass,
      CxdFeeTable::PriceClass priceClass, CxdFeeTable::BboType bboType) {
    auto bboIndex = static_cast<std::size_t>(bboType);
    auto priceIndex = static_cast<std::size_t>(priceClass);
    auto liquidityIndex = static_cast<std::size_t>(liquidityFlag);
    auto columnIndex = bboIndex * CxdFeeTable::PRICE_CLASS_COUNT + priceIndex;
    if(securityClass == CxdFeeTable::SecurityClass::ETF) {
      return feeTable.m_etfFeeTable[liquidityIndex][columnIndex];
    }
    return feeTable.m_feeTable[liquidityIndex][columnIndex];
  }

  /**
   * Looks up the max fee in the CXD fee table.
   * @param feeTable The CxdFeeTable to use.
   * @param liquidityFlag The LiquidityFlag (ACTIVE or PASSIVE).
   * @param priceClass The PriceClass (SUBDOLLAR, SUBFIVE, DEFAULT).
   * @param bboType The BboType (AT_BBO or INSIDE_BBO).
   * @return The max fee for the specified parameters.
   */
  inline Money LookupMaxFee(const CxdFeeTable& feeTable,
      LiquidityFlag liquidityFlag, CxdFeeTable::PriceClass priceClass,
      CxdFeeTable::BboType bboType) {
    auto bboIndex = static_cast<std::size_t>(bboType);
    auto priceIndex = static_cast<std::size_t>(priceClass);
    auto liquidityIndex = static_cast<std::size_t>(liquidityFlag);
    auto columnIndex = bboIndex * CxdFeeTable::PRICE_CLASS_COUNT + priceIndex;
    return feeTable.m_maxFeeTable[liquidityIndex][columnIndex];
  }

  /**
   * Calculates the fee for a trade executed on CXD.
   * @param feeTable The CxdFeeTable to use.
   * @param securityClass The SecurityClass (DEFAULT or ETF).
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee for the specified trade.
   */
  inline Money CalculateFee(const CxdFeeTable& feeTable,
      CxdFeeTable::SecurityClass securityClass,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto bboType = GetCxdBboType(executionReport.m_liquidityFlag);
    auto liquidityFlag = GetCxdLiquidityFlag(executionReport.m_liquidityFlag);
    if(bboType == CxdFeeTable::BboType::NONE ||
        liquidityFlag == LiquidityFlag::NONE) {
      std::cout << "Unknown liquidity flag [CXD]: \"" <<
        executionReport.m_liquidityFlag << "\"\n";
      bboType = CxdFeeTable::BboType::AT_BBO;
      liquidityFlag = LiquidityFlag::ACTIVE;
    }
    auto priceClass = GetCxdPriceClass(executionReport.m_lastPrice);
    auto perShareFee =
      LookupFee(feeTable, liquidityFlag, securityClass, priceClass, bboType);
    if(securityClass == CxdFeeTable::SecurityClass::ETF) {
      return executionReport.m_lastQuantity * perShareFee;
    } else {
      auto maxFee = LookupMaxFee(feeTable, liquidityFlag, priceClass, bboType);
      return std::min(executionReport.m_lastQuantity * perShareFee, maxFee);
    }
  }
}

#endif
