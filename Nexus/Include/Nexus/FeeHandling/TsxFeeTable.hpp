#ifndef NEXUS_TSX_FEE_TABLE_HPP
#define NEXUS_TSX_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by TSX. */
  struct TsxFeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price < $0.10. */
      SUBDIME = 0,

      /** Price >= $0.10 & < $1.00. */
      SUBDOLLAR,

      /** Price >= $1.00 and non-interlisted. */
      DEFAULT,

      /** Price >= $1.00 and interlisted. */
      DEFAULT_INTERLISTED,

      /** Price >= $1.00 and ETF. */
      DEFAULT_ETF
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(5);

    /** Enumerates the types of trades. */
    enum class Type {

      /** Unknown. */
      NONE = -1,

      /** Active. */
      ACTIVE = 0,

      /** Passive. */
      PASSIVE,

      /** Hidden active. */
      HIDDEN_ACTIVE,

      /** Hidden passive. */
      HIDDEN_PASSIVE
    };

    /** The number of Type enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(4);

    /** Enumerates the indices into an auction fee table. */
    enum class AuctionIndex {

      /** Unknown. */
      NONE = -1,

      /** The fee. */
      FEE = 0,

      /** The fee cap. */
      MAX_CHARGE
    };

    /** The number of AuctionIndices enumerated. */
    static constexpr auto AUCTION_INDEX_COUNT = std::size_t(2);

    /** Enumerates the types of auctions. */
    enum class AuctionType {

      /** Unknown. */
      NONE = -1,

      /** Opening auction. */
      OPEN = 0,

      /** Closing auction. */
      CLOSE
    };

    /** The number of AuctionTypes enumerated. */
    static constexpr auto AUCTION_TYPE_COUNT = std::size_t(2);

    /** Enumerates various classifications for a Security. */
    enum class Classification {

      /** Unknown. */
      NONE = -1,

      /** The default classification. */
      DEFAULT = 0,

      /** An ETF. */
      ETF,

      /** An interlisted symbol. */
      INTERLISTED
    };

    /** The number of Classifications enumerated. */
    static constexpr auto CLASSIFICATION_COUNT = std::size_t(3);

    /** The continuous fee table. */
    std::array<std::array<Money, PRICE_CLASS_COUNT>, TYPE_COUNT>
      m_continuous_fee_table;

    /** The auction fee table. */
    std::array<std::array<Money, AUCTION_INDEX_COUNT>, AUCTION_TYPE_COUNT>
      m_auction_fee_table;

    /** The odd-lot list. */
    std::array<Money, 3> m_odd_lot_fee_list;
  };

  /**
   * Parses a TsxFeeTable from a YAML configuration.
   * @param config The configuration to parse the TsxFeeTable from.
   * @return The TsxFeeTable represented by the <i>config</i>.
   */
  inline TsxFeeTable parse_tsx_fee_table(const YAML::Node& config) {
    auto table = TsxFeeTable();
    parse_fee_table(
      config, "continuous_fee_table", Beam::out(table.m_continuous_fee_table));
    parse_fee_table(
      config, "auction_fee_table", Beam::out(table.m_auction_fee_table));
    parse_fee_table(
      config, "odd_lot_list", Beam::out(table.m_odd_lot_fee_list));
    return table;
  }

  /**
   * Looks up a fee in the continuous fee table.
   * @param table The TsxFeeTable used to lookup the fee.
   * @param price_class The trade's PriceClass.
   * @param type The Type of trade.
   * @return The fee corresponding to the specified <i>price_class</i> and
   *         <i>type</i>.
   */
  inline Money lookup_continuous_fee(const TsxFeeTable& table,
      TsxFeeTable::PriceClass price_class, TsxFeeTable::Type type) {
    return table.m_continuous_fee_table[static_cast<int>(type)][
      static_cast<int>(price_class)];
  }

  /**
   * Looks up a fee in the auction fee table.
   * @param table The TsxFeeTable used to lookup the fee.
   * @param index The AuctionIndex to lookup.
   * @param type The AuctionType to lookup.
   * @return The fee corresponding to the specified <i>index</i> and
   *         <i>type</i>.
   */
  inline Money lookup_auction_fee(const TsxFeeTable& table,
      TsxFeeTable::AuctionIndex index, TsxFeeTable::AuctionType type) {
    return table.m_auction_fee_table[static_cast<int>(type)][
      static_cast<int>(index)];
  }

  /**
   * Looks up a fee in the odd lot list.
   * @param table The TsxFeeTable used to lookup the fee.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>price_class</i>.
   */
  inline Money lookup_odd_lot_fee(
      const TsxFeeTable& table, TsxFeeTable::PriceClass price_class) {
    return table.m_odd_lot_fee_list[static_cast<int>(price_class)];
  }

  /**
   * Returns <code>true</code> iff an OrderFields represents a hidden TSX
   * Order.
   * @param order The Order to test.
   * @return <code>true</code> iff the <i>order</i> is classified as a hidden
   *         TSX Order.
   */
  inline bool is_tsx_hidden_order(const OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED;
  }

  /**
   * Calculates the fee on a trade executed on TSX.
   * @param table The TsxFeeTable used to calculate the fee.
   * @param classification The Security's classification.
   * @param order The Order that executed the trade.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(const TsxFeeTable& table,
      TsxFeeTable::Classification classification, const OrderFields& fields,
      const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    if(report.m_last_quantity < 100) {
      auto price_class = [&] {
        if(report.m_last_price < 10 * Money::CENT) {
          return TsxFeeTable::PriceClass::SUBDIME;
        } else if(report.m_last_price < Money::ONE) {
          return TsxFeeTable::PriceClass::SUBDOLLAR;
        } else {
          return TsxFeeTable::PriceClass::DEFAULT;
        }
      }();
      auto fee = lookup_odd_lot_fee(table, price_class);
      return report.m_last_quantity * fee;
    } else if(report.m_liquidity_flag.size() == 1) {
      if(report.m_liquidity_flag == "O") {
        auto fee = lookup_auction_fee(table, TsxFeeTable::AuctionIndex::FEE,
          TsxFeeTable::AuctionType::OPEN);
        auto max_fee = lookup_auction_fee(
          table, TsxFeeTable::AuctionIndex::MAX_CHARGE,
          TsxFeeTable::AuctionType::OPEN);
        return std::min(report.m_last_quantity * fee, max_fee);
      } else {
        auto price_class = [&] {
          if(report.m_last_price < 10 * Money::CENT) {
            return TsxFeeTable::PriceClass::SUBDIME;
          } else if(report.m_last_price < Money::ONE) {
            return TsxFeeTable::PriceClass::SUBDOLLAR;
          } else if(classification == TsxFeeTable::Classification::DEFAULT) {
            return TsxFeeTable::PriceClass::DEFAULT;
          } else if(classification == TsxFeeTable::Classification::ETF) {
            return TsxFeeTable::PriceClass::DEFAULT_ETF;
          } else if(classification == TsxFeeTable::Classification::INTERLISTED) {
            return TsxFeeTable::PriceClass::DEFAULT_INTERLISTED;
          } else {
            std::cout << "Unknown classification [TSX].\n";
            return TsxFeeTable::PriceClass::DEFAULT;
          }
        }();
        auto type = [&] {
          if(report.m_liquidity_flag[0] == 'P') {
            if(is_tsx_hidden_order(fields)) {
              return TsxFeeTable::Type::HIDDEN_PASSIVE;
            } else {
              return TsxFeeTable::Type::PASSIVE;
            }
          } else if(report.m_liquidity_flag[0] == 'A') {
            if(is_tsx_hidden_order(fields)) {
              return TsxFeeTable::Type::HIDDEN_ACTIVE;
            } else {
              return TsxFeeTable::Type::ACTIVE;
            }
          } else {
            std::cout << "Unknown liquidity flag [TSX]: \"" <<
              report.m_liquidity_flag << "\".\n";
            return TsxFeeTable::Type::ACTIVE;
          }
        }();
        auto fee = lookup_continuous_fee(table, price_class, type);
        return report.m_last_quantity * fee;
      }
    } else if(report.m_liquidity_flag.size() == 2) {
      auto type = [&] {
        if(report.m_liquidity_flag == "AO" || report.m_liquidity_flag == "AE") {
          return TsxFeeTable::AuctionType::CLOSE;
        } else {
          std::cout << "Unknown liquidity flag [TSX]: \"" <<
            report.m_liquidity_flag << "\".\n";
          return TsxFeeTable::AuctionType::CLOSE;
        }
      }();
      auto fee =
        lookup_auction_fee(table, TsxFeeTable::AuctionIndex::FEE, type);
      auto max_fee =
        lookup_auction_fee(table, TsxFeeTable::AuctionIndex::MAX_CHARGE, type);
      return std::min(report.m_last_quantity * fee, max_fee);
    } else {
      std::cout << "Unknown liquidity flag [TSX]: \"" <<
        report.m_liquidity_flag << "\".\n";
      return Money::ZERO;
    }
  }
}

#endif
