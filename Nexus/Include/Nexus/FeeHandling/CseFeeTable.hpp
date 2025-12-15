#ifndef NEXUS_CSE_FEE_TABLE_HPP
#define NEXUS_CSE_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
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
      TRADE_TYPE_COUNT * PRICE_CLASS_COUNT> m_fee_table;

    /** The fee table for interlisted continuous trading. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>,
      TRADE_TYPE_COUNT * PRICE_CLASS_COUNT> m_interlisted_fee_table;

    /** The fee table for ETF listed continuous trading. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>,
      TRADE_TYPE_COUNT * PRICE_CLASS_COUNT> m_etf_fee_table;

    /** The fee table for CSE listed continuous trading. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_cse_listed_fee_table;

    /** The fee table for CSE listed opening trades. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_cse_open_fee_table;

    /** The fee table for CSE listed closing trades. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_cse_close_fee_table;
  };

  /**
   * Parses a CseFeeTable from a YAML configuration.
   * @param config The configuration to parse the CseFeeTable from.
   * @return The CseFeeTable represented by the <i>config</i>.
   */
  inline CseFeeTable parse_cse_fee_table(const YAML::Node& config) {
    auto table = CseFeeTable();
    parse_fee_table(config, "fee_table", Beam::out(table.m_fee_table));
    parse_fee_table(config, "interlisted_fee_table",
      Beam::out(table.m_interlisted_fee_table));
    parse_fee_table(config, "etf_fee_table", Beam::out(table.m_etf_fee_table));
    parse_fee_table(
      config, "cse_listed_fee_table", Beam::out(table.m_cse_listed_fee_table));
    parse_fee_table(
      config, "open_fee_table", Beam::out(table.m_cse_open_fee_table));
    parse_fee_table(
      config, "close_fee_table", Beam::out(table.m_cse_close_fee_table));
    return table;
  }

  /**
   * Determines the liquidity flag for a CSE trade based on the provided string.
   * @param flag The liquidity flag string.
   * @return The corresponding LiquidityFlag.
   */
  inline LiquidityFlag get_cse_liquidity_flag(const std::string& flag) {
    if(flag.size() >= 1) {
      if(flag[0] == 'P') {
        return LiquidityFlag::PASSIVE;
      } else if(flag[0] == 'T') {
        return LiquidityFlag::ACTIVE;
      } else {
        std::cout << "Unknown liquidity flag [CSE]: \"" << flag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    } else {
      std::cout << "Unknown liquidity flag [CSE]: \"" << flag << "\"\n";
      return LiquidityFlag::ACTIVE;
    }
  }

  /**
   * Classifies a trade price into a CSE price class.
   * @param price The trade price.
   * @return The corresponding PriceClass.
   */
  inline CseFeeTable::PriceClass get_cse_price_class(Money price) {
    if(price < Money::ONE) {
      return CseFeeTable::PriceClass::SUBDOLLAR;
    }
    return CseFeeTable::PriceClass::DEFAULT;
  }

  /**
   * Determines the session type for a CSE trade based on the provided string.
   * @param flag The liquidity flag string.
   * @return The corresponding Session.
   */
  inline CseFeeTable::Session get_cse_session(const std::string& flag) {
    if(flag.size() > 3) {
      if(flag[3] == 'O') {
        return CseFeeTable::Session::OPEN;
      } else if(flag[3] == 'M') {
        return CseFeeTable::Session::CLOSE;
      }
    }
    return CseFeeTable::Session::DEFAULT;
  }

  /**
   * Determines the trade type for a CSE trade based on the provided string.
   * @param flag The liquidity flag string.
   * @return The corresponding TradeType.
   */
  inline CseFeeTable::TradeType get_cse_trade_type(const std::string& flag) {
    if(flag.size() >= 3 && flag[2] == 'D') {
      return CseFeeTable::TradeType::DARK;
    }
    return CseFeeTable::TradeType::DEFAULT;
  }

  /**
   * Retrieves the fee for a default security based on trade type, price class,
   * and liquidity flag.
   * @param table The CseFeeTable containing the fee data.
   * @param type The trade type.
   * @param price_class The price class of the trade.
   * @param flag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money get_default_fee(const CseFeeTable& table,
      CseFeeTable::TradeType type, CseFeeTable::PriceClass price_class,
      LiquidityFlag flag) {
    auto row = static_cast<std::size_t>(type) *
      CseFeeTable::PRICE_CLASS_COUNT + static_cast<std::size_t>(price_class);
    return table.m_fee_table[row][static_cast<std::size_t>(flag)];
  }

  /**
   * Retrieves the fee for an interlisted security based on trade type, price
   * class, and liquidity flag.
   * @param table The CseFeeTable containing the fee data.
   * @param type The trade type.
   * @param price_class The price class of the trade.
   * @param flag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money get_interlisted_fee(const CseFeeTable& table,
      CseFeeTable::TradeType type, CseFeeTable::PriceClass price_class,
      LiquidityFlag flag) {
    auto row = static_cast<std::size_t>(type) *
      CseFeeTable::PRICE_CLASS_COUNT + static_cast<std::size_t>(price_class);
    return table.m_interlisted_fee_table[row][static_cast<std::size_t>(flag)];
  }

  /**
   * Retrieves the fee for an ETF security based on trade type, price class, and
   * liquidity flag.
   * @param table The CseFeeTable containing the fee data.
   * @param type The trade type.
   * @param price_class The price class of the trade.
   * @param flag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money get_etf_fee(const CseFeeTable& table,
      CseFeeTable::TradeType type, CseFeeTable::PriceClass price_class,
      LiquidityFlag flag) {
    auto row = static_cast<std::size_t>(type) *
      CseFeeTable::PRICE_CLASS_COUNT + static_cast<std::size_t>(price_class);
    return table.m_etf_fee_table[row][static_cast<std::size_t>(flag)];
  }

  /**
   * Retrieves the fee for a CSE listed security based on price class and
   * liquidity flag.
   * @param table The CseFeeTable containing the fee data.
   * @param price_class The price class of the trade.
   * @param flag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money get_cse_listed_fee(const CseFeeTable& table,
      CseFeeTable::PriceClass price_class, LiquidityFlag flag) {
    return table.m_cse_listed_fee_table[static_cast<std::size_t>(price_class)][
      static_cast<std::size_t>(flag)];
  }

  /**
   * Retrieves the fee for a CSE listed security in the opening auction based on
   * price class and liquidity flag.
   * @param table The CseFeeTable containing the fee data.
   * @param price_class The price class of the trade.
   * @param flag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money get_open_fee(const CseFeeTable& table,
      CseFeeTable::PriceClass price_class, LiquidityFlag flag) {
    return table.m_cse_open_fee_table[static_cast<std::size_t>(price_class)][
      static_cast<std::size_t>(flag)];
  }

  /**
   * Retrieves the fee for a CSE listed security in the closing auction based on
   * price class and liquidity flag.
   * @param table The CseFeeTable containing the fee data.
   * @param price_class The price class of the trade.
   * @param flag The liquidity flag of the trade.
   * @return The corresponding fee.
   */
  inline Money get_close_fee(const CseFeeTable& table,
      CseFeeTable::PriceClass price_class, LiquidityFlag flag) {
    return table.m_cse_close_fee_table[static_cast<std::size_t>(price_class)][
      static_cast<std::size_t>(flag)];
  }

  /**
   * Calculates the fee for a trade executed on the CSE.
   * @param table The CseFeeTable containing the fee data.
   * @param listing The CseListing type of the security.
   * @param report The ExecutionReport for the trade.
   * @return The calculated fee for the trade.
   */
  inline Money calculate_fee(const CseFeeTable& table,
      CseFeeTable::CseListing listing, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto price_class = get_cse_price_class(report.m_last_price);
    auto flag = get_cse_liquidity_flag(report.m_liquidity_flag);
    auto session = get_cse_session(report.m_liquidity_flag);
    if(session == CseFeeTable::Session::OPEN) {
      return report.m_last_quantity * get_open_fee(table, price_class, flag);
    } else if(session == CseFeeTable::Session::CLOSE) {
      return report.m_last_quantity * get_close_fee(table, price_class, flag);
    }
    auto type = get_cse_trade_type(report.m_liquidity_flag);
    auto fee = [&] {
      if(listing == CseFeeTable::CseListing::CSE_LISTED) {
        return get_cse_listed_fee(table, price_class, flag);
      } else if(listing == CseFeeTable::CseListing::INTERLISTED) {
        return get_interlisted_fee(table, type, price_class, flag);
      } else if(listing == CseFeeTable::CseListing::ETF) {
        return get_etf_fee(table, type, price_class, flag);
      }
      return get_default_fee(table, type, price_class, flag);
    }();
    return report.m_last_quantity * fee;
  }
}

#endif
