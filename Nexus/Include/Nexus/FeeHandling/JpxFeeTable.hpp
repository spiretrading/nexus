#ifndef NEXUS_JPX_FEE_TABLE_HPP
#define NEXUS_JPX_FEE_TABLE_HPP
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by JPX markets. */
  struct JpxFeeTable {

    /** Fee charged for the software. */
    Money m_spireFee;

    /** The brokerage fee. */
    boost::rational<int> m_brokerageFee;
  };

  /**
   * Parses a JpxFeeTable from a YAML configuration.
   * @param config The configuration to parse the JpxFeeTable from.
   * @param marketDatabase The MarketDatabase used to parse Securities.
   * @return The JpxFeeTable represented by the <i>config</i>.
   */
  inline JpxFeeTable ParseJpxFeeTable(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    auto feeTable = JpxFeeTable();
    feeTable.m_spireFee = Beam::Extract<Money>(config, "spire_fee");
    feeTable.m_brokerageFee = Beam::Extract<boost::rational<int>>(config,
      "brokerage_fee");
    return feeTable;
  }

  /**
   * Calculates the fee on a trade executed on a JPX market.
   * @param feeTable The JpxFeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return An ExecutionReport containing the calculated fees.
   */
  inline OrderExecutionService::ExecutionReport CalculateFee(
      const JpxFeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return executionReport;
    }
    auto feesReport = executionReport;
    feesReport.m_commission += feeTable.m_spireFee;
    auto notionalValue = feesReport.m_lastQuantity * feesReport.m_lastPrice;
    feesReport.m_processingFee += feeTable.m_brokerageFee * notionalValue;
    return feesReport;
  }
}

#endif
