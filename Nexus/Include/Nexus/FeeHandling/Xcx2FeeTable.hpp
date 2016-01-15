#ifndef NEXUS_XCX2FEETABLE_HPP
#define NEXUS_XCX2FEETABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /*! \struct Xcx2FeeTable
      \brief Stores the table of fees used by CX2 on TSX.
   */
  struct Xcx2FeeTable {

    /*! \enum PriceClass
        \brief Enumerates the types of price classes.
     */
    enum class PriceClass : int {

      //! Unknown.
      NONE = -1,

      //! Price >= $1.00.
      DEFAULT,

      //! Price < $1.00.
      SUB_DOLLAR,
    };

    //! The number of price classes enumerated.
    static const std::size_t PRICE_CLASS_COUNT = 2;

    /*! \enum Type
        \brief Enumerates the types of trades.
     */
    enum class Type : int {

      //! Unknown.
      NONE = -1,

      //! Active.
      ACTIVE = 0,

      //! Passive.
      PASSIVE,

      //! Large Size Trade Passive.
      LARGE_PASSIVE,

      //! Hidden Active.
      HIDDEN_ACTIVE,

      //! Hidden Passive.
      HIDDEN_PASSIVE,

      //! Odd-lot.
      ODD_LOT
    };

    //! The number of trade types enumerated.
    static const std::size_t TYPE_COUNT = 6;

    //! The fee table.
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT> m_feeTable;

    //! The large trade size threshold.
    Quantity m_largeTradeSize;

    //! The large trade size threshold for sub-dollar trades.
    Quantity m_largeTradeSizeSubDollar;
  };

  //! Parses an Xcx2FeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the Xcx2FeeTable from.
    \return The Xcx2FeeTable represented by the <i>config</i>.
  */
  inline Xcx2FeeTable ParseXcx2FeeTable(const YAML::Node& config) {
    Xcx2FeeTable feeTable;
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    feeTable.m_largeTradeSize = Beam::Extract<Quantity>(config,
      "large_trade_size");
    feeTable.m_largeTradeSizeSubDollar = Beam::Extract<Quantity>(config,
      "large_trade_size_sub_dollar");
    return feeTable;
  }

  //! Looks up a fee.
  /*!
    \param feeTable The Xcx2FeeTable used to lookup the fee.
    \param type The trade's Type.
    \param priceClass The trade's PriceClass.
    \return The fee corresponding to the specified <i>type</i> and
            <i>priceClass</i>.
  */
  inline Money LookupFee(const Xcx2FeeTable& feeTable, Xcx2FeeTable::Type type,
      Xcx2FeeTable::PriceClass priceClass) {
    return feeTable.m_feeTable[static_cast<int>(priceClass)][
      static_cast<int>(type)];
  }

  //! Tests if an OrderFields represents a hidden liquidity provider.
  /*!
    \param fields The OrderFields to test.
    \return <code>true</code> iff the <i>order</i> counts as a hidden liquidity
            provider.
  */
  inline bool IsXcx2HiddenLiquidityProvider(
      const OrderExecutionService::OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      OrderExecutionService::HasField(fields, Tag{18, "M"});
  }

  //! Calculates the fee on a trade executed on XCX2.
  /*!
    \param feeTable The Xcx2FeeTable used to calculate the fee.
    \param orderFields The OrderFields  the trade took place on.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return The fee calculated for the specified trade.
  */
  inline Money CalculateFee(const Xcx2FeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < Money::ONE) {
        return Xcx2FeeTable::PriceClass::SUB_DOLLAR;
      } else {
        return Xcx2FeeTable::PriceClass::DEFAULT;
      }
    }();
    auto type = [&] {
      if(executionReport.m_lastQuantity < 100) {
        return Xcx2FeeTable::Type::ODD_LOT;
      } else if(IsXcx2HiddenLiquidityProvider(fields)) {
        if(executionReport.m_liquidityFlag.size() == 1) {
          if(executionReport.m_liquidityFlag[0] == 'A') {
            return Xcx2FeeTable::Type::HIDDEN_ACTIVE;
          } else if(executionReport.m_liquidityFlag[0] == 'P') {
            return Xcx2FeeTable::Type::HIDDEN_PASSIVE;
          } else {
            std::cout << "Unknown liquidity flag [XCX2]: \"" <<
              executionReport.m_liquidityFlag << "\"\n";
            return Xcx2FeeTable::Type::HIDDEN_PASSIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [XCX2]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return Xcx2FeeTable::Type::HIDDEN_PASSIVE;
        }
      } else {
        if(executionReport.m_liquidityFlag.size() == 1) {
          if(executionReport.m_liquidityFlag[0] == 'A') {
            return Xcx2FeeTable::Type::ACTIVE;
          } else if(executionReport.m_liquidityFlag[0] == 'P') {
            if((priceClass == Xcx2FeeTable::PriceClass::SUB_DOLLAR &&
                executionReport.m_lastQuantity >=
                feeTable.m_largeTradeSizeSubDollar) ||
                (priceClass == Xcx2FeeTable::PriceClass::DEFAULT &&
                executionReport.m_lastQuantity >= feeTable.m_largeTradeSize)) {
              return Xcx2FeeTable::Type::LARGE_PASSIVE;
            } else {
              return Xcx2FeeTable::Type::PASSIVE;
            }
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
      }
    }();
    auto fee = LookupFee(feeTable, type, priceClass);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
