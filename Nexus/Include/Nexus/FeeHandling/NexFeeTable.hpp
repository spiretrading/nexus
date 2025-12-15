#ifndef NEXUS_NEX_FEE_TABLE_HPP
#define NEXUS_NEX_FEE_TABLE_HPP
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
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
  inline NexFeeTable parse_nex_fee_table(const YAML::Node& config) {
    auto table = NexFeeTable();
    table.m_fee = Beam::extract<Money>(config, "fee");
    return table;
  }

  /**
   * Calculates the fee on a trade executed on NEX.
   * @param table The NexFeeTable used to calculate the fee.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(
      const NexFeeTable& table, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    return report.m_last_quantity * table.m_fee;
  }
}

#endif
