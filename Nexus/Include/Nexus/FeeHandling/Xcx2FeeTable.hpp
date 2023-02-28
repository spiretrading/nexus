#ifndef NEXUS_XCX2_FEE_TABLE_HPP
#define NEXUS_XCX2_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by CX2 on TSX. */
  struct Xcx2FeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price >= $5.00. */
      DEFAULT,

      /** ETFs ($1.00 or over). */
      ETF,

      /** Price < $5.00. */
      SUB_FIVE_DOLLAR,

      /** Price < $1.00. */
      SUBDOLLAR,

      /** Price < $0.10. */
      SUBDIME,
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

      /** Large Size Trade Active. */
      LARGE_ACTIVE,

      /** Large Size Trade Passive. */
      LARGE_PASSIVE,

      /** Hidden Active. */
      HIDDEN_ACTIVE,

      /** Hidden Passive. */
      HIDDEN_PASSIVE,

      /** Odd-lot. */
      ODD_LOT
    };

    /** The number of trade types enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(7);

    /** The fee table for non-TSX listed securities. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT> m_defaultTable;

    /** The fee table for TSX listed securities. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT> m_tsxTable;

    /** The large trade size threshold. */
    Quantity m_largeTradeSize;

    /** The set of ETFs. */
    std::unordered_set<Security> m_etfs;
  };

  /**
   * Parses an Xcx2FeeTable from a YAML configuration.
   * @param config The configuration to parse the Xcx2FeeTable from.
   * @param etfs The set of ETF Securities.
   * @return The Xcx2FeeTable represented by the <i>config</i>.
   */
  inline Xcx2FeeTable ParseXcx2FeeTable(const YAML::Node& config,
      std::unordered_set<Security> etfs) {
    auto feeTable = Xcx2FeeTable();
    ParseFeeTable(config, "default_table",
      Beam::Store(feeTable.m_defaultTable));
    ParseFeeTable(config, "tsx_table", Beam::Store(feeTable.m_tsxTable));
    feeTable.m_largeTradeSize =
      Beam::Extract<Quantity>(config, "large_trade_size");
    feeTable.m_etfs = std::move(etfs);
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The Xcx2FeeTable used to lookup the fee.
   * @param orderFields The OrderFields the trade took place on.
   * @param type The trade's Type.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>type</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupFee(const Xcx2FeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields, Xcx2FeeTable::Type type,
      Xcx2FeeTable::PriceClass priceClass) {
    if(fields.m_security.GetMarket() == DefaultMarkets::TSX()) {
      return feeTable.m_tsxTable[static_cast<int>(priceClass)][
        static_cast<int>(type)];
    } else {
      return feeTable.m_defaultTable[static_cast<int>(priceClass)][
        static_cast<int>(type)];
    }
  }

  /**
   * Calculates the fee on a trade executed on XCX2.
   * @param feeTable The Xcx2FeeTable used to calculate the fee.
   * @param orderFields The OrderFields the trade took place on.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const Xcx2FeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto priceClass = [&] {
      if(feeTable.m_etfs.count(fields.m_security) == 1) {
        return Xcx2FeeTable::PriceClass::ETF;
      } else if(executionReport.m_lastPrice < 10 * Money::CENT) {
        return Xcx2FeeTable::PriceClass::SUBDIME;
      } else if(executionReport.m_lastPrice < Money::ONE) {
        return Xcx2FeeTable::PriceClass::SUBDOLLAR;
      } else if (executionReport.m_lastPrice < 5 * Money::ONE) {
        return Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR;
      } else {
        return Xcx2FeeTable::PriceClass::DEFAULT;
      }
    }();
    auto type = [&] {
      if(executionReport.m_lastQuantity < 100) {
        return Xcx2FeeTable::Type::ODD_LOT;
      } else if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P' ||
            executionReport.m_liquidityFlag[0] == 'S') {
          if(executionReport.m_lastQuantity >= feeTable.m_largeTradeSize) {
            return Xcx2FeeTable::Type::LARGE_PASSIVE;
          } else {
            return Xcx2FeeTable::Type::PASSIVE;
          }
        } else if(executionReport.m_liquidityFlag[0] == 'A' ||
            executionReport.m_liquidityFlag[0] == 'C') {
          if(executionReport.m_lastQuantity >= feeTable.m_largeTradeSize) {
            return Xcx2FeeTable::Type::LARGE_ACTIVE;
          } else {
            return Xcx2FeeTable::Type::ACTIVE;
          }
        } else if(executionReport.m_liquidityFlag[0] == 'a' ||
            executionReport.m_liquidityFlag[0] == 'd') {
          return Xcx2FeeTable::Type::HIDDEN_PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'r' ||
            executionReport.m_liquidityFlag[0] == 'D') {
          return Xcx2FeeTable::Type::HIDDEN_ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [XCX2]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return Xcx2FeeTable::Type::PASSIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [XCX2]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return Xcx2FeeTable::Type::PASSIVE;
      }
    }();
    auto fee = LookupFee(feeTable, fields, type, priceClass);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
