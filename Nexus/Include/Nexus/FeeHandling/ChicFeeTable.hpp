#ifndef NEXUS_CHICFEETABLE_HPP
#define NEXUS_CHICFEETABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /*! \struct ChicFeeTable
      \brief Stores the table of fees used by CHI-X on TSX.
   */
  struct ChicFeeTable {

    /*! \enum Category
        \brief Enumerates the fee categories.
     */
    enum class Category : int {

      //! Unknown.
      NONE = -1,

      //! Price >= $1.00.
      DEFAULT,

      //! Price < $1.00.
      SUB_DOLLAR,

      //! ETF
      ETF
    };

    //! The number of categories enumerated.
    static const std::size_t CATEGORY_COUNT = 3;

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

      //! Passive hidden
      HIDDEN
    };

    //! The number of trade types enumerated.
    static const std::size_t TYPE_COUNT = 3;

    //! The fee table.
    std::array<std::array<Money, TYPE_COUNT>, CATEGORY_COUNT> m_feeTable;
  };

  //! Parses a ChicFeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the ChicFeeTable from.
    \return The ChicFeeTable represented by the <i>config</i>.
  */
  inline ChicFeeTable ParseChicFeeTable(const YAML::Node& config) {
    ChicFeeTable feeTable;
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    return feeTable;
  }

  //! Looks up a fee.
  /*!
    \param feeTable The ChicFeeTable used to lookup the fee.
    \param type The trade's type.
    \param category The trade's Category.
    \return The fee corresponding to the specified <i>type</i> and
            <i>category</i>.
  */
  inline Money LookupFee(const ChicFeeTable& feeTable,
      ChicFeeTable::Type type, ChicFeeTable::Category category) {
    return feeTable.m_feeTable[static_cast<int>(category)][
      static_cast<int>(type)];
  }

  //! Tests if an OrderFields represents a hidden liquidity provider.
  /*!
    \param fields The OrderFields to test.
    \return <code>true</code> iff the <i>order</i> counts as a hidden liquidity
            provider.
  */
  inline bool IsChicHiddenLiquidityProvider(
      const OrderExecutionService::OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      OrderExecutionService::HasField(fields, Tag{18, "M"});
  }

  //! Calculates the fee on a trade executed on CHIC.
  /*!
    \param feeTable The ChicFeeTable used to calculate the fee.
    \param isEtf Whether the calculation is for an ETF.
    \param fields The OrderFields used to place the Order.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return The fee calculated for the specified trade.
  */
  inline Money CalculateFee(const ChicFeeTable& feeTable, bool isEtf,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto category = [&] {
      if(executionReport.m_lastPrice < Money::ONE) {
        return ChicFeeTable::Category::SUB_DOLLAR;
      } else if(isEtf) {
        return ChicFeeTable::Category::ETF;
      } else {
        return ChicFeeTable::Category::DEFAULT;
      }
    }();
    auto type = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P') {
          if(IsChicHiddenLiquidityProvider(fields)) {
            return ChicFeeTable::Type::HIDDEN;
          } else {
            return ChicFeeTable::Type::PASSIVE;
          }
        } else if(executionReport.m_liquidityFlag[0] == 'A') {
          return ChicFeeTable::Type::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [CHIC]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return ChicFeeTable::Type::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [CHIC]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return ChicFeeTable::Type::ACTIVE;
      }
    }();
    auto fee = LookupFee(feeTable, type, category);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
