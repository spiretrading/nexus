#ifndef NEXUS_LYNX_FEE_TABLE_HPP
#define NEXUS_LYNX_FEE_TABLE_HPP
#include <array>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by the LYNX ATS. */
  struct LynxFeeTable {

    /** Lists the available trade classifications. */
    enum class Classification {

      /** Unknown. */
      NONE = -1,

      /** Price >= $1.00 and not interlisted. */
      DEFAULT,

      /** Price >= $1.00 and interlisted. */
      INTERLISTED,

      /** ETFs. */
      ETF,

      /** Price < $1.00. */
      SUBDOLLAR,

      /** Midpoint execution. */
      MIDPOINT,
    };

    /** The number of price classes enumerated. */
    static constexpr auto CLASSIFICATION_COUNT = std::size_t(5);

    /** The fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, CLASSIFICATION_COUNT>
      m_feeTable;

    /** The set of interlisted securities. */
    std::unordered_set<Security> m_interlisted;

    /** The set of ETFs. */
    std::unordered_set<Security> m_etfs;
  };

  /**
   * Parses a LynxFeeTable from a YAML configuration.
   * @param config The configuration to parse the LynxFeeTable from.
   * @param interlisted The set of interlisted Securities.
   * @param etfs The set of ETF Securities.
   * @return The LynxFeeTable represented by the <i>config</i>.
   */
  inline LynxFeeTable ParseLynxFeeTable(const YAML::Node& config,
      std::unordered_set<Security> etfs,
      std::unordered_set<Security> interlisted) {
    auto feeTable = LynxFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    feeTable.m_interlisted = std::move(interlisted);
    feeTable.m_etfs = std::move(etfs);
    return feeTable;
  }

  /**
   * Tests if an OrderFields represents a midpoint order on LYNX.
   * @param fields The OrderFields to test.
   * @return <code>true</code> iff the <i>order</i> counts as a midpoint order
   * on LYNX.
   */
  inline bool IsLynxMidpointOrder(
      const OrderExecutionService::OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      OrderExecutionService::HasField(fields, Tag(18, "M"));
  }

  /**
   * Looks up a fee.
   * @param feeTable The LynxFeeTable used to lookup the fee.
   * @param liquidityFlag The liquidity flag to lookup.
   * @param classification The trade's classification.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag,
      LynxFeeTable::Classification classification) {
    return feeTable.m_feeTable[static_cast<int>(classification)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Calculates the fee on a trade executed on LYNX.
   * @param feeTable The LynxFeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const LynxFeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto liquidityFlag = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P') {
          return LiquidityFlag::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'A') {
          return LiquidityFlag::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [LYNX]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return LiquidityFlag::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [LYNX]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    }();
    auto classification = [&] {
      if(IsLynxMidpointOrder(fields)) {
        return LynxFeeTable::Classification::MIDPOINT;
      } else if(executionReport.m_lastPrice < Money::ONE) {
        return LynxFeeTable::Classification::SUBDOLLAR;
      } else if(feeTable.m_interlisted.count(fields.m_security) == 1) {
        return LynxFeeTable::Classification::INTERLISTED;
      } else if(feeTable.m_etfs.count(fields.m_security) == 1) {
        return LynxFeeTable::Classification::ETF;
      } else {
        return LynxFeeTable::Classification::DEFAULT;
      }
    }();
    auto fee = LookupFee(feeTable, liquidityFlag, classification);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
