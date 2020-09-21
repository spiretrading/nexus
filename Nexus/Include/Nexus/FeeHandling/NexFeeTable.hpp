#ifndef NEXUS_NEX_FEE_TABLE_HPP
#define NEXUS_NEX_FEE_TABLE_HPP
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by NEX listed securities. */
  struct NexFeeTable {

    /** The fee per share. */
    Money m_fee;
  };

  /**
   * Parses an NexFeeTable from a YAML configuration.
   * @param config The configuration to parse the NexFeeTable from.
   * @return The NexFeeTable represented by the <i>config</i>.
   */
  inline NexFeeTable ParseNexFeeTable(const YAML::Node& config) {
    auto feeTable = NexFeeTable();
    feeTable.m_fee = Beam::Extract<Money>(config, "fee");
    return feeTable;
  }

  /**
   * Calculates the fee on a trade executed on NEX.
   * @param feeTable The NexFeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const NexFeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    return executionReport.m_lastQuantity * feeTable.m_fee;
  }
}

#endif
