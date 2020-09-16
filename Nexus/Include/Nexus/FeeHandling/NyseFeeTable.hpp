#ifndef NEXUS_NYSE_FEE_TABLE_HPP
#define NEXUS_NYSE_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by NYSE. */
  struct NyseFeeTable {

    /** Enumerates the category of trades. */
    enum class Category {

      /** Unknown. */
      NONE = -1,

      /** Default category. */
      DEFAULT = 0,

      /** Hidden order. */
      HIDDEN,

      /** Cross order. */
      CROSS,

      /** On open. */
      ON_OPEN,

      /** On close. */
      ON_CLOSE,

      /** Retail order. */
      RETAIL
    };

    /** The number of trade categories enumerated. */
    static constexpr auto CATEGORY_COUNT = std::size_t(6);

    /** The fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, CATEGORY_COUNT>
      m_feeTable;

    /** The subdollar rates. */
    std::array<boost::rational<int>, LIQUIDITY_FLAG_COUNT> m_subdollarTable;
  };

  /**
   * Parses a NyseFeeTable from a YAML configuration.
   * @param config The configuration to parse the NyseFeeTable from.
   * @return The NyseFeeTable represented by the <i>config</i>.
   */
  inline NyseFeeTable ParseNyseFeeTable(const YAML::Node& config) {
    auto feeTable = NyseFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    ParseFeeTable(config, "subdollar_table",
      Beam::Store(feeTable.m_subdollarTable));
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The NyseFeeTable used to lookup the fee.
   * @param flag The trade's liquidity flag.
   * @param category The trade's Category.
   * @return The fee corresponding to the specified <i>flag</i> and
   *         <i>category</i>.
   */
  inline Money LookupFee(const NyseFeeTable& feeTable, LiquidityFlag flag,
      NyseFeeTable::Category category) {
    return feeTable.m_feeTable[static_cast<int>(category)][
      static_cast<int>(flag)];
  }

  /**
   * Tests if an OrderFields represents a hidden liquidity provider.
   * @param fields The OrderFields to test.
   * @return <code>true</code> iff the <i>order</i> counts as a hidden liquidity
   *         provider.
   */
  inline bool IsNyseHiddenLiquidityProvider(
      const OrderExecutionService::OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      OrderExecutionService::HasField(fields, Tag{18, "M"});
  }

  /**
   * Calculates the fee on a trade executed on NYSE.
   * @param feeTable The NyseFeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const NyseFeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto isHidden = IsNyseHiddenLiquidityProvider(fields);
    auto flag = LiquidityFlag::ACTIVE;
    auto category = NyseFeeTable::Category::DEFAULT;
    if(executionReport.m_liquidityFlag.size() == 1) {
      if(executionReport.m_liquidityFlag[0] == '1') {
        flag = LiquidityFlag::ACTIVE;
        if(isHidden) {
          category = NyseFeeTable::Category::HIDDEN;
        }
      } else if(executionReport.m_liquidityFlag[0] == '2') {
        flag = LiquidityFlag::PASSIVE;
        if(isHidden) {
          category = NyseFeeTable::Category::HIDDEN;
        }
      } else if(executionReport.m_liquidityFlag[0] == '3') {
        flag = LiquidityFlag::ACTIVE;
      } else if(executionReport.m_liquidityFlag[0] == '5') {
        flag = LiquidityFlag::PASSIVE;
        category = NyseFeeTable::Category::ON_OPEN;
      } else if(executionReport.m_liquidityFlag[0] == '6') {
        flag = LiquidityFlag::ACTIVE;
        category = NyseFeeTable::Category::ON_CLOSE;
      } else if(executionReport.m_liquidityFlag[0] == '7') {
        flag = LiquidityFlag::ACTIVE;
        category = NyseFeeTable::Category::ON_CLOSE;
      } else if(executionReport.m_liquidityFlag[0] == '8') {
        flag = LiquidityFlag::PASSIVE;
        category = NyseFeeTable::Category::RETAIL;
      } else if(executionReport.m_liquidityFlag[0] == '9') {
        flag = LiquidityFlag::ACTIVE;
        category = NyseFeeTable::Category::RETAIL;
      } else {
        std::cout << "Unknown liquidity flag [NYSE]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
      }
    } else {
      std::cout << "Unknown liquidity flag [NYSE]: \"" <<
        executionReport.m_liquidityFlag << "\"\n";
    }
    if(executionReport.m_lastPrice < Money::ONE) {
      auto rate = feeTable.m_subdollarTable[static_cast<int>(flag)];
      return rate *
        (executionReport.m_lastQuantity * executionReport.m_lastPrice);
    }
    auto fee = LookupFee(feeTable, flag, category);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
