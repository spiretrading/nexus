#ifndef NEXUS_NSDQ_FEE_TABLE_HPP
#define NEXUS_NSDQ_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by NSDQ. */
  struct NsdqFeeTable {

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
   * Parses a NsdqFeeTable from a YAML configuration.
   * @param config The configuration to parse the NsdqFeeTable from.
   * @return The NsdqFeeTable represented by the <i>config</i>.
   */
  inline NsdqFeeTable ParseNsdqFeeTable(const YAML::Node& config) {
    auto feeTable = NsdqFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    ParseFeeTable(config, "subdollar_table",
      Beam::Store(feeTable.m_subdollarTable));
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The NyseFeeTable used to lookup the fee.
   * @param flag The liquidity flag.
   * @param category The trade's Category.
   * @return The fee corresponding to the specified <i>flag</i> and
   *         <i>category</i>.
   */
  inline Money LookupFee(const NsdqFeeTable& feeTable, LiquidityFlag flag,
      NsdqFeeTable::Category category) {
    return feeTable.m_feeTable[static_cast<int>(category)][
      static_cast<int>(flag)];
  }

  /**
   * Calculates the fee on a trade executed on NSDQ.
   * @param feeTable The NsdqFeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const NsdqFeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto flag = LiquidityFlag::ACTIVE;
    auto category = NsdqFeeTable::Category::DEFAULT;
    if(executionReport.m_liquidityFlag.size() == 1) {
      if(executionReport.m_liquidityFlag[0] == 'A' ||
          executionReport.m_liquidityFlag[0] == '7' ||
          executionReport.m_liquidityFlag[0] == '8' ||
          executionReport.m_liquidityFlag[0] == 'a' ||
          executionReport.m_liquidityFlag[0] == 'x' ||
          executionReport.m_liquidityFlag[0] == 'y' ||
          executionReport.m_liquidityFlag[0] == '0') {
        flag = LiquidityFlag::PASSIVE;
        category = NsdqFeeTable::Category::DEFAULT;
      } else if(executionReport.m_liquidityFlag[0] == 'X') {
        flag = LiquidityFlag::ACTIVE;
        category = NsdqFeeTable::Category::DEFAULT;
      } else if(executionReport.m_liquidityFlag[0] == 'e') {
        flag = LiquidityFlag::PASSIVE;
        category = NsdqFeeTable::Category::RETAIL;
      } else if(executionReport.m_liquidityFlag[0] == 'k' ||
          executionReport.m_liquidityFlag[0] == 'J') {
        flag = LiquidityFlag::PASSIVE;
        category = NsdqFeeTable::Category::HIDDEN;
      } else if(executionReport.m_liquidityFlag[0] == 'R') {
        flag = LiquidityFlag::ACTIVE;
        category = NsdqFeeTable::Category::DEFAULT;
      } else if(executionReport.m_liquidityFlag[0] == 'm') {
        flag = LiquidityFlag::ACTIVE;
        category = NsdqFeeTable::Category::HIDDEN;
      } else if(executionReport.m_liquidityFlag[0] == 'O' ||
          executionReport.m_liquidityFlag[0] == 'M') {
        flag = LiquidityFlag::ACTIVE;
        category = NsdqFeeTable::Category::ON_OPEN;
      } else if(executionReport.m_liquidityFlag[0] == 'C' ||
          executionReport.m_liquidityFlag[0] == 'L') {
        flag = LiquidityFlag::ACTIVE;
        category = NsdqFeeTable::Category::ON_CLOSE;
      } else {
        std::cout << "Unknown liquidity flag [NSDQ]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
      }
    } else {
      std::cout << "Unknown liquidity flag [NSDQ]: \"" <<
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
