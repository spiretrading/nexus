#ifndef NEXUS_CSE_FEE_TABLE_HPP
#define NEXUS_CSE_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by the CSE. */
  struct CseFeeTable {

    /** Enumerates the types of listings. */
    enum class CseListing {

      /** Used for general securities. */
      DEFAULT,

      /** Used for interlisted securities. */
      INTERLISTED,

      /** Used for ETF listed securities. */
      ETF,

      /** Used for CSE listed securities. */
      CSE_LISTED
    };

    /** The number of listings enumerated. */
    static const auto LISTING_COUNT = std::size_t(4);

    /** Enumerates the price classes. */
    enum class PriceClass {

      /** Trade with a price greater than or equal to $1.00. */
      DEFAULT,

      /** Trade with a price less than $1.00. */
      SUBDOLLAR
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(2);

    /** Enumerates the types of trading sessions. */
    enum class Session {

      /** Default/continuous trading session. */
      DEFAULT,

      /** Opening auction. */
      OPEN,

      /** Closing auction. */
      CLOSE
    };

    /** The number of session types enumerated. */
    static const auto SESSION_COUNT = std::size_t(3);

    /** Lists the types of trades. */
    enum class TradeType {

      /** Default trade. */
      DEFAULT,

      /** Dark trade. */
      DARK
    };

    /** The number of trade types enumerated. */
    static constexpr auto TRADE_TYPE_COUNT = std::size_t(2);

    /** The fee table for default listed continuous trading. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>,
      TRADE_TYPE_COUNT * PRICE_CLASS_COUNT> m_feeTable;

    /** The fee table for interlisted continuous trading. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>,
      TRADE_TYPE_COUNT * PRICE_CLASS_COUNT> m_interlistedFeeTable;

    /** The fee table for ETF listed continuous trading. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>,
      TRADE_TYPE_COUNT * PRICE_CLASS_COUNT> m_etfFeeTable;

    /** The fee table for CSE listed continuous trading. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_cseListedFeeTable;

    /** The fee table for CSE listed opening trades. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_cseOpenFeeTable;

    /** The fee table for CSE listed closing trades. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_cseCloseFeeTable;
  };

  /**
   * Parses a CseFeeTable from a YAML configuration.
   * @param config The configuration to parse the CseFeeTable from.
   * @return The CseFeeTable represented by the <i>config</i>.
   */
  inline CseFeeTable ParseCseFeeTable(const YAML::Node& config) {
    auto feeTable = CseFeeTable();
    ParseFeeTable(config, "fee_table",
      Beam::Store(feeTable.m_feeTable));
    ParseFeeTable(config, "interlisted_fee_table",
      Beam::Store(feeTable.m_interlistedFeeTable));
    ParseFeeTable(config, "etf_fee_table",
      Beam::Store(feeTable.m_etfFeeTable));
    ParseFeeTable(config, "cse_listed_fee_table",
      Beam::Store(feeTable.m_cseListedFeeTable));
    ParseFeeTable(config, "open_fee_table",
      Beam::Store(feeTable.m_cseOpenFeeTable));
    ParseFeeTable(config, "close_fee_table",
      Beam::Store(feeTable.m_cseCloseFeeTable));
    return feeTable;
  }

  /**
   * Determines the liquidity flag for a CSE trade based on the provided string.
   * @param liquidityFlag The liquidity flag string.
   * @return The corresponding LiquidityFlag.
   */
  inline LiquidityFlag GetCseLiquidityFlag(const std::string& liquidityFlag) {
    if(liquidityFlag.size() >= 1) {
      if(liquidityFlag[0] == 'P') {
        return LiquidityFlag::PASSIVE;
      } else if(liquidityFlag[0] == 'T') {
        return LiquidityFlag::ACTIVE;
      } else {
        std::cout << "Unknown liquidity flag [CSE]: \"" <<
          liquidityFlag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    } else {
      std::cout << "Unknown liquidity flag [CSE]: \"" <<
        liquidityFlag << "\"\n";
      return LiquidityFlag::ACTIVE;
    }
  }

  /**
   * Classifies a trade price into a CSE price class.
   * @param price The trade price.
   * @return The corresponding PriceClass.
   */
  inline CseFeeTable::PriceClass GetCsePriceClass(Money price) {
    if(price < Money::ONE) {
      return CseFeeTable::PriceClass::SUBDOLLAR;
    }
    return CseFeeTable::PriceClass::DEFAULT;
  }

  /**
   * Determines the session type for a CSE trade based on the provided string.
   * @param liquidityFlag The liquidity flag string.
   * @return The corresponding Session.
   */
  inline CseFeeTable::Session GetCseSession(const std::string& liquidityFlag) {
    if(liquidityFlag.size() > 3) {
      if(liquidityFlag[3] == 'O') {
        return CseFeeTable::Session::OPEN;
      } else if(liquidityFlag[3] == 'M') {
        return CseFeeTable::Session::CLOSE;
      }
    }
    return CseFeeTable::Session::DEFAULT;
  }

  /**
   * Determines the trade type for a CSE trade based on the provided string.
   * @param liquidityFlag The liquidity flag string.
   * @return The corresponding TradeType.
   */
  inline CseFeeTable::TradeType GetCseTradeType(
      const std::string& liquidityFlag) {
    if(liquidityFlag.size() >= 3 && liquidityFlag[2] == 'D') {
      return CseFeeTable::TradeType::DARK;
    }
    return CseFeeTable::TradeType::DEFAULT;
  }

  /**
   * Retrieves the fee for a default security based on trade type, price class,
   * and liquidity flag.
   * @param feeTable The CseFeeTable containing the fee data.
   * @param tradeType The trade type.
   * @param priceClass The price class of the trade.
   * @param liquidityFlag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money GetDefaultFee(const CseFeeTable& feeTable,
      CseFeeTable::TradeType tradeType, CseFeeTable::PriceClass priceClass,
      LiquidityFlag liquidityFlag) {
    auto row = static_cast<std::size_t>(tradeType) *
      CseFeeTable::PRICE_CLASS_COUNT + static_cast<std::size_t>(priceClass);
    return feeTable.m_feeTable[row][static_cast<std::size_t>(liquidityFlag)];
  }

  /**
   * Retrieves the fee for an interlisted security based on trade type, price
   * class, and liquidity flag.
   * @param feeTable The CseFeeTable containing the fee data.
   * @param tradeType The trade type.
   * @param priceClass The price class of the trade.
   * @param liquidityFlag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money GetInterlistedFee(const CseFeeTable& feeTable,
      CseFeeTable::TradeType tradeType, CseFeeTable::PriceClass priceClass,
      LiquidityFlag liquidityFlag) {
    auto row = static_cast<std::size_t>(tradeType) *
      CseFeeTable::PRICE_CLASS_COUNT + static_cast<std::size_t>(priceClass);
    return feeTable.m_interlistedFeeTable[row][
      static_cast<std::size_t>(liquidityFlag)];
  }

  /**
   * Retrieves the fee for an ETF security based on trade type, price class, and
   * liquidity flag.
   * @param feeTable The CseFeeTable containing the fee data.
   * @param tradeType The trade type.
   * @param priceClass The price class of the trade.
   * @param liquidityFlag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money GetEtfFee(const CseFeeTable& feeTable,
      CseFeeTable::TradeType tradeType, CseFeeTable::PriceClass priceClass,
      LiquidityFlag liquidityFlag) {
    auto row = static_cast<std::size_t>(tradeType) *
      CseFeeTable::PRICE_CLASS_COUNT + static_cast<std::size_t>(priceClass);
    return feeTable.m_etfFeeTable[row][static_cast<std::size_t>(liquidityFlag)];
  }

  /**
   * Retrieves the fee for a CSE listed security based on price class and
   * liquidity flag.
   * @param feeTable The CseFeeTable containing the fee data.
   * @param priceClass The price class of the trade.
   * @param liquidityFlag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money GetCseListedFee(const CseFeeTable& feeTable,
      CseFeeTable::PriceClass priceClass, LiquidityFlag liquidityFlag) {
    return feeTable.m_cseListedFeeTable[static_cast<std::size_t>(priceClass)][
      static_cast<std::size_t>(liquidityFlag)];
  }

  /**
   * Retrieves the fee for a CSE listed security in the opening auction based on
   * price class and liquidity flag.
   * @param feeTable The CseFeeTable containing the fee data.
   * @param priceClass The price class of the trade.
   * @param liquidityFlag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money GetOpenFee(const CseFeeTable& feeTable,
      CseFeeTable::PriceClass priceClass, LiquidityFlag liquidityFlag) {
    return feeTable.m_cseOpenFeeTable[static_cast<std::size_t>(priceClass)][
      static_cast<std::size_t>(liquidityFlag)];
  }

  /**
   * Retrieves the fee for a CSE listed security in the closing auction based on
   * price class and liquidity flag.
   * @param feeTable The CseFeeTable containing the fee data.
   * @param priceClass The price class of the trade.
   * @param liquidityFlag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money GetCloseFee(const CseFeeTable& feeTable,
      CseFeeTable::PriceClass priceClass, LiquidityFlag liquidityFlag) {
    return feeTable.m_cseCloseFeeTable[static_cast<std::size_t>(priceClass)][
      static_cast<std::size_t>(liquidityFlag)];
  }

  /**
   * Calculates the fee for a trade executed on the CSE.
   * @param feeTable The CseFeeTable containing the fee data.
   * @param listing The CseListing type of the security.
   * @param executionReport The ExecutionReport for the trade.
   * @return The calculated fee for the trade.
   */
  inline Money CalculateFee(const CseFeeTable& feeTable,
      CseFeeTable::CseListing listing,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto priceClass = GetCsePriceClass(executionReport.m_lastPrice);
    auto liquidityFlag = GetCseLiquidityFlag(executionReport.m_liquidityFlag);
    auto session = GetCseSession(executionReport.m_liquidityFlag);
    if(session == CseFeeTable::Session::OPEN) {
      return executionReport.m_lastQuantity *
        GetOpenFee(feeTable, priceClass, liquidityFlag);
    } else if(session == CseFeeTable::Session::CLOSE) {
      return executionReport.m_lastQuantity *
        GetCloseFee(feeTable, priceClass, liquidityFlag);
    }
    auto tradeType = GetCseTradeType(executionReport.m_liquidityFlag);
    auto fee = [&] {
      if(listing == CseFeeTable::CseListing::CSE_LISTED) {
        return GetCseListedFee(feeTable, priceClass, liquidityFlag);
      } else if(listing == CseFeeTable::CseListing::INTERLISTED) {
        return GetInterlistedFee(feeTable, tradeType, priceClass, liquidityFlag);
      } else if(listing == CseFeeTable::CseListing::ETF) {
        return GetEtfFee(feeTable, tradeType, priceClass, liquidityFlag);
      }
      return GetDefaultFee(feeTable, tradeType, priceClass, liquidityFlag);
    }();
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
