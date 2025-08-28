#ifndef NEXUS_TSX_FEE_TABLE_HPP
#define NEXUS_TSX_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
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
      MAX_CHARGE,
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
      CLOSE,
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
      m_continuousFeeTable;

    /** The auction fee table. */
    std::array<std::array<Money, AUCTION_INDEX_COUNT>, AUCTION_TYPE_COUNT>
      m_auctionFeeTable;

    /** The odd-lot list. */
    std::array<Money, 3> m_oddLotFeeList;
  };

  /**
   * Parses a TsxFeeTable from a YAML configuration.
   * @param config The configuration to parse the TsxFeeTable from.
   * @return The TsxFeeTable represented by the <i>config</i>.
   */
  inline TsxFeeTable ParseTsxFeeTable(const YAML::Node& config) {
    auto feeTable = TsxFeeTable();
    ParseFeeTable(config, "continuous_fee_table",
      Beam::Store(feeTable.m_continuousFeeTable));
    ParseFeeTable(config, "auction_fee_table",
      Beam::Store(feeTable.m_auctionFeeTable));
    ParseFeeTable(config, "odd_lot_list",
      Beam::Store(feeTable.m_oddLotFeeList));
    return feeTable;
  }

  /**
   * Looks up a fee in the continuous fee table.
   * @param feeTable The TsxFeeTable used to lookup the fee.
   * @param priceClass The trade's PriceClass.
   * @param type The Type of trade.
   * @return The fee corresponding to the specified <i>priceClass</i> and
   *         <i>type</i>.
   */
  inline Money LookupContinuousFee(const TsxFeeTable& feeTable,
      TsxFeeTable::PriceClass priceClass, TsxFeeTable::Type type) {
    return feeTable.m_continuousFeeTable[static_cast<int>(type)][
      static_cast<int>(priceClass)];
  }

  /**
   * Looks up a fee in the auction fee table.
   * @param feeTable The TsxFeeTable used to lookup the fee.
   * @param auctionIndex The AuctionIndex to lookup.
   * @param auctionType The AuctionType to lookup.
   * @return The fee corresponding to the specified <i>auctionIndex</i> and
   *         <i>auctionType</i>.
   */
  inline Money LookupAuctionFee(const TsxFeeTable& feeTable,
      TsxFeeTable::AuctionIndex auctionIndex,
      TsxFeeTable::AuctionType auctionType) {
    return feeTable.m_auctionFeeTable[static_cast<int>(auctionType)][
      static_cast<int>(auctionIndex)];
  }

  /**
   * Looks up a fee in the odd lot list.
   * @param feeTable The TsxFeeTable used to lookup the fee.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>priceClass</i>.
   */
  inline Money LookupOddLotFee(const TsxFeeTable& feeTable,
      TsxFeeTable::PriceClass priceClass) {
    return feeTable.m_oddLotFeeList[static_cast<int>(priceClass)];
  }

  /**
   * Returns <code>true</code> iff an OrderFields represents a hidden TSX
   * Order.
   * @param order The Order to test.
   * @return <code>true</code> iff the <i>order</i> is classified as a hidden
   *         TSX Order.
   */
  inline bool IsTsxHiddenOrder(
      const OrderExecutionService::OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED;
  }

  /**
   * Calculates the fee on a trade executed on TSX.
   * @param feeTable The TsxFeeTable used to calculate the fee.
   * @param classification The Security's classification.
   * @param order The Order that executed the trade.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const TsxFeeTable& feeTable,
      TsxFeeTable::Classification classification,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    if(executionReport.m_lastQuantity < 100) {
      auto priceClass = [&] {
        if(executionReport.m_lastPrice < 10 * Money::CENT) {
          return TsxFeeTable::PriceClass::SUBDIME;
        } else if(executionReport.m_lastPrice < Money::ONE) {
          return TsxFeeTable::PriceClass::SUBDOLLAR;
        } else {
          return TsxFeeTable::PriceClass::DEFAULT;
        }
      }();
      auto fee = LookupOddLotFee(feeTable, priceClass);
      return executionReport.m_lastQuantity * fee;
    } else if(executionReport.m_liquidityFlag.size() == 1) {
      if(executionReport.m_liquidityFlag == "O") {
        auto fee = LookupAuctionFee(feeTable, TsxFeeTable::AuctionIndex::FEE,
          TsxFeeTable::AuctionType::OPEN);
        auto maxFee = LookupAuctionFee(
          feeTable, TsxFeeTable::AuctionIndex::MAX_CHARGE,
          TsxFeeTable::AuctionType::OPEN);
        return std::min(executionReport.m_lastQuantity * fee, maxFee);
      } else {
        auto priceClass = [&] {
          if(executionReport.m_lastPrice < 10 * Money::CENT) {
            return TsxFeeTable::PriceClass::SUBDIME;
          } else if(executionReport.m_lastPrice < Money::ONE) {
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
          if(executionReport.m_liquidityFlag[0] == 'P') {
            if(IsTsxHiddenOrder(fields)) {
              return TsxFeeTable::Type::HIDDEN_PASSIVE;
            } else {
              return TsxFeeTable::Type::PASSIVE;
            }
          } else if(executionReport.m_liquidityFlag[0] == 'A') {
            if(IsTsxHiddenOrder(fields)) {
              return TsxFeeTable::Type::HIDDEN_ACTIVE;
            } else {
              return TsxFeeTable::Type::ACTIVE;
            }
          } else {
            std::cout << "Unknown liquidity flag [TSX]: \"" <<
              executionReport.m_liquidityFlag << "\".\n";
            return TsxFeeTable::Type::ACTIVE;
          }
        }();
        auto fee = LookupContinuousFee(feeTable, priceClass, type);
        return executionReport.m_lastQuantity * fee;
      }
    } else if(executionReport.m_liquidityFlag.size() == 2) {
      auto type = [&] {
        if(executionReport.m_liquidityFlag == "AO" ||
            executionReport.m_liquidityFlag == "AE") {
          return TsxFeeTable::AuctionType::CLOSE;
        } else {
          std::cout << "Unknown liquidity flag [TSX]: \"" <<
            executionReport.m_liquidityFlag << "\".\n";
          return TsxFeeTable::AuctionType::CLOSE;
        }
      }();
      auto fee = LookupAuctionFee(feeTable, TsxFeeTable::AuctionIndex::FEE,
        type);
      auto maxFee = LookupAuctionFee(feeTable,
        TsxFeeTable::AuctionIndex::MAX_CHARGE, type);
      return std::min(executionReport.m_lastQuantity * fee, maxFee);
    } else {
      std::cout << "Unknown liquidity flag [TSX]: \"" <<
        executionReport.m_liquidityFlag << "\".\n";
      return Money::ZERO;
    }
  }
}

#endif
