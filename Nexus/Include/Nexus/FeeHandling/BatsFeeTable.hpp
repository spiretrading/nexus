#ifndef NEXUS_BATS_FEE_TABLE_HPP
#define NEXUS_BATS_FEE_TABLE_HPP
#include <unordered_map>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by BATS BYX. */
  struct BatsFeeTable {

    /** Maps liquidity flags to per share fee rates. */
    std::unordered_map<std::string, boost::rational<int>> m_feeTable;

    /** The default liquidity flag to use. */
    std::string m_defaultFlag;
  };

  /**
   * Parses a BatsFeeTable from a YAML configuration.
   * @param config The configuration to parse the BatsFeeTable from.
   * @return The BatsFeeTable represented by the <i>config</i>.
   */
  inline BatsFeeTable ParseBatsFeeTable(const YAML::Node& config) {
    auto feeTable = BatsFeeTable();
    ParseFeeTable(config, "liquidity_flags", Beam::Store(feeTable.m_feeTable));
    feeTable.m_defaultFlag = Beam::Extract<std::string>(config, "default");
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The BatsFeeTable used to lookup the fee.
   * @param flag The liquidity flag to lookup.
   * @return The fee corresponding to the specified <i>flag</i>.
   */
  inline boost::rational<int> LookupFee(const BatsFeeTable& feeTable,
      const std::string& flag) {
    auto feeIterator = feeTable.m_feeTable.find(flag);
    if(feeIterator != feeTable.m_feeTable.end()) {
      return feeIterator->second;
    }
    auto defaultIterator = feeTable.m_feeTable.find(feeTable.m_defaultFlag);
    if(defaultIterator != feeTable.m_feeTable.end()) {
      return defaultIterator->second;
    }
    return boost::rational<int>{0, 1};
  }

  /**
   * Calculates the fee on a trade executed on BATS.
   * @param feeTable The BatsFeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const BatsFeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto fee = LookupFee(feeTable, executionReport.m_liquidityFlag);
    if(executionReport.m_lastPrice < Money::ONE) {
      return fee *
        (executionReport.m_lastQuantity * executionReport.m_lastPrice);
    } else {
      return (fee * executionReport.m_lastQuantity) * Money::ONE;
    }
  }
}

#endif
