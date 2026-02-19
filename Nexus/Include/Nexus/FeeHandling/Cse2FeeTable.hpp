#ifndef NEXUS_CSE2_FEE_TABLE_HPP
#define NEXUS_CSE2_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by the CSE2 destination. */
  struct Cse2FeeTable {

    /** Enumerates the sections within the fee table. */
    enum class Section {

      /** Regular fee table. */
      REGULAR,

      /** Dark fee table. */
      DARK,

      /** Debentures or notes table. */
      DEBENTURES_OR_NOTES,

      /** CSE Government listed bond table. */
      CSE_LISTED_GOVERNMENT_BONDS,

      /** Oddlot table. */
      ODDLOT
    };

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Price >= $1.00. */
      DEFAULT,

      /** Price >= $0.10 & < $1.00. */
      SUBDOLLAR
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(2);

    /** The market the security is listed on. */
    enum class ListingMarket {

      /** CSE listed. */
      CSE,

      /** TSX or TSX-V listed. */
      TSX_TSXV
    };

    /** The number of listings. */
    static constexpr auto LISTING_MARKET_COUNT = std::size_t(2);

    /** The regular fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_regular_table;

    /** The dark order fees. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_dark_table;

    /** The debentures or notes table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, LISTING_MARKET_COUNT>
      m_debentures_or_notes_table;

    /** The CSE listed government bonds fees. */
    std::array<Money, LIQUIDITY_FLAG_COUNT> m_cse_listed_government_bonds_table;

    /** The oddlot fees. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_oddlot_table;

    /** The large trade size threshold. */
    Quantity m_large_trade_size;

    /** The large trade fee applied when volume >= threshold. */
    Money m_large_trade_fee;
  };

  /**
   * Parses a Cse2FeeTable from a YAML configuration.
   * @param config The configuration to parse the CseFeeTable from.
   * @return The Cse2FeeTable represented by the <i>config</i>.
   */
  inline Cse2FeeTable parse_cse2_fee_table(const YAML::Node& config) {
    auto table = Cse2FeeTable();
    parse_fee_table(config, "regular_table", Beam::out(table.m_regular_table));
    parse_fee_table(config, "dark_table", Beam::out(table.m_dark_table));
    parse_fee_table(config, "debentures_or_notes_table",
      Beam::out(table.m_debentures_or_notes_table));
    parse_fee_table(config, "cse_listed_government_bonds_table",
      Beam::out(table.m_cse_listed_government_bonds_table));
    parse_fee_table(config, "oddlot_table", Beam::out(table.m_oddlot_table));
    table.m_large_trade_size =
      Beam::extract<Quantity>(config, "large_trade_size");
    table.m_large_trade_fee = Beam::extract<Money>(config, "large_trade_fee");
    return table;
  }

  /**
   * Determines what section of the CSE2 fee table is needed to calculate a fee.
   * @param fields The OrderFields used to place the Order.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The section within the <i>Cse2FeeTable</i> needed to calculate the
   *         fee for the specified <i>report</i>.
   */
  inline Cse2FeeTable::Section lookup_cse2_fee_table_section(
      const OrderFields& fields, const ExecutionReport& report) {
    if(report.m_liquidity_flag.size() >= 3 &&
        report.m_liquidity_flag[2] == 'D') {
      return Cse2FeeTable::Section::DARK;
    } else if(report.m_last_quantity < 100) {
      return Cse2FeeTable::Section::ODDLOT;
    } else if(fields.m_ticker.get_symbol().find(".DB") != std::string::npos ||
        fields.m_ticker.get_symbol().find(".NT") != std::string::npos ||
        fields.m_ticker.get_symbol().find(".NO") != std::string::npos ||
        fields.m_ticker.get_symbol().find(".NS") != std::string::npos) {
      return Cse2FeeTable::Section::DEBENTURES_OR_NOTES;
    }
    return Cse2FeeTable::Section::REGULAR;
  }

  /**
   * Returns the price class of a CSE2 execution report.
   * @param report An ExecutionReport received from CSE2.
   * @return The price class for the given <i>report</i>.
   */
  inline Cse2FeeTable::PriceClass lookup_cse2_price_class(
      const ExecutionReport& report) {
    if(report.m_last_price < Money::ONE) {
      return Cse2FeeTable::PriceClass::SUBDOLLAR;
    }
    return Cse2FeeTable::PriceClass::DEFAULT;
  }

  /**
   * Returns the liquidity flag for a CSE2 execution report.
   * @param report An ExecutionReport received from CSE2.
   * @return The <i>report</i>'s liquidity flag.
   */
  inline LiquidityFlag lookup_cse2_liquidity_flag(
      const ExecutionReport& report) {
    if(report.m_liquidity_flag.empty() || report.m_liquidity_flag[0] == 'P') {
      return LiquidityFlag::PASSIVE;
    }
    return LiquidityFlag::ACTIVE;
  }

  /**
   * Returns the listing market for a CSE2 execution report.
   * @param report An ExecutionReport received from CSE2.
   * @return The <i>report</i>'s listing market.
   */
  inline Cse2FeeTable::ListingMarket lookup_cse2_listing_market(
      const ExecutionReport& report) {
    if(report.m_liquidity_flag.size() <= 1 ||
        report.m_liquidity_flag[1] == 'T' ||
        report.m_liquidity_flag[1] == 'V' ||
        report.m_liquidity_flag[1] == 'N') {
      return Cse2FeeTable::ListingMarket::TSX_TSXV;
    }
    return Cse2FeeTable::ListingMarket::CSE;
  }

  /**
   * Looks up a fee from the CSE2 regular fee table.
   * @param table The Cse2FeeTable to look up.
   * @param flag The fee's liquidity flag.
   * @param price_class The fee's price class.
   * @return The regular fee table entry.
   */
  inline Money lookup_regular_fee(const Cse2FeeTable& table, LiquidityFlag flag,
      Cse2FeeTable::PriceClass price_class) {
    return table.m_regular_table[static_cast<int>(price_class)][
      static_cast<int>(flag)];
  }

  /**
   * Calculates a regular fee executed on CSE2.
   * @param table The Cse2FeeTable used to calculate the fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_regular_fee(
      const Cse2FeeTable& table, const ExecutionReport& report) {
    auto price_class = lookup_cse2_price_class(report);
    auto flag = lookup_cse2_liquidity_flag(report);
    auto fee = lookup_regular_fee(table, flag, price_class);
    if(report.m_last_quantity >= table.m_large_trade_size) {
      return table.m_large_trade_fee;
    }
    return report.m_last_quantity * fee;
  }

  /**
   * Looks up a fee from the CSE2 dark fee table.
   * @param table The Cse2FeeTable to look up.
   * @param flag The fee's liquidity flag.
   * @param price_class The fee's price class.
   * @return The dark fee table entry.
   */
  inline Money lookup_dark_fee(const Cse2FeeTable& table,
      LiquidityFlag flag, Cse2FeeTable::PriceClass price_class) {
    return table.m_dark_table[static_cast<int>(price_class)][
      static_cast<int>(flag)];
  }

  /**
   * Calculates a dark fee executed on CSE2.
   * @param table The Cse2FeeTable used to calculate the fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_dark_fee(
      const Cse2FeeTable& table, const ExecutionReport& report) {
    auto price_class = lookup_cse2_price_class(report);
    auto flag = lookup_cse2_liquidity_flag(report);
    auto fee = lookup_dark_fee(table, flag, price_class);
    return report.m_last_quantity * fee;
  }

  /**
   * Looks up the fee entry from a CSE2 debentures or notes table.
   * @param table The Cse2FeeTable to look up.
   * @param flag The fee's liquidity flag.
   * @param market The listing market.
   * @return The debentures or notes table entry.
   */
  inline Money lookup_debentures_or_notes_fee(const Cse2FeeTable& table,
      LiquidityFlag flag, Cse2FeeTable::ListingMarket market) {
    return table.m_debentures_or_notes_table[
      static_cast<int>(market)][static_cast<int>(flag)];
  }

  /**
   * Calculates a debentures or notes fee executed on CSE2.
   * @param table The Cse2FeeTable used to calculate the fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_debentures_or_notes_fee(
      const Cse2FeeTable& table, const ExecutionReport& report) {
    auto flag = lookup_cse2_liquidity_flag(report);
    auto market = lookup_cse2_listing_market(report);
    auto fee = lookup_debentures_or_notes_fee(table, flag, market);
    if(report.m_last_quantity >= table.m_large_trade_size) {
      return table.m_large_trade_fee;
    }
    return report.m_last_quantity * fee;
  }

  /**
   * Looks up the fee entry from a CSE2 CSE listed government bonds fee table.
   * @param table The Cse2FeeTable to look up.
   * @param flag The fee's liquidity flag.
   * @return The CSE listed government bons fee table entry.
   */
  inline Money lookup_cse_listed_government_bonds_fee(
      const Cse2FeeTable& table, LiquidityFlag flag) {
    return table.m_cse_listed_government_bonds_table[static_cast<int>(flag)];
  }

  /**
   * Calculates a CSE listed government bond fee executed on CSE2.
   * @param table The Cse2FeeTable used to calculate the fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_cse_listed_government_bonds_fee(
      const Cse2FeeTable& table, const ExecutionReport& report) {
    auto flag = lookup_cse2_liquidity_flag(report);
    auto fee = lookup_cse_listed_government_bonds_fee(table, flag);
    if(report.m_last_quantity >= table.m_large_trade_size) {
      return table.m_large_trade_fee;
    }
    return report.m_last_quantity * fee;
  }

  /**
   * Looks up the fee entry from a CSE2 oddlot fee table.
   * @param table The Cse2FeeTable to look up.
   * @param price_class The fee's price class.
   * @param flag The fee's liquidity flag.
   * @return The oddlot table entry.
   */
  inline Money lookup_oddlot_fee(const Cse2FeeTable& table, LiquidityFlag flag,
      Cse2FeeTable::PriceClass price_class) {
    return table.m_oddlot_table[static_cast<int>(price_class)][
      static_cast<int>(flag)];
  }

  /**
   * Calculates an oddlot fee executed on CSE2.
   * @param table The Cse2FeeTable used to calculate the fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_oddlot_fee(
      const Cse2FeeTable& table, const ExecutionReport& report) {
    auto flag = lookup_cse2_liquidity_flag(report);
    auto price_class = lookup_cse2_price_class(report);
    auto fee = lookup_oddlot_fee(table, flag, price_class);
    return report.m_last_quantity * fee;
  }

  /**
   * Calculates the fee on a trade executed on CSE2.
   * @param table The Cse2FeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(const Cse2FeeTable& table,
      const OrderFields& fields, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto section = lookup_cse2_fee_table_section(fields, report);
    if(section == Cse2FeeTable::Section::REGULAR) {
      return calculate_regular_fee(table, report);
    } else if(section == Cse2FeeTable::Section::DARK) {
      return calculate_dark_fee(table, report);
    } else if(section == Cse2FeeTable::Section::DEBENTURES_OR_NOTES) {
      return calculate_debentures_or_notes_fee(table, report);
    } else if(section == Cse2FeeTable::Section::CSE_LISTED_GOVERNMENT_BONDS) {
      return calculate_cse_listed_government_bonds_fee(table, report);
    } else if(section == Cse2FeeTable::Section::ODDLOT) {
      return calculate_oddlot_fee(table, report);
    }
    std::cout << "Unknown liquidity flag [CSE2]: \"" <<
      report.m_liquidity_flag << "\"\n";
    return calculate_regular_fee(table, report);
  }
}

#endif
