#ifndef NEXUS_AMEX_FEE_TABLE_HPP
#define NEXUS_AMEX_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by AMEX. */
  struct AmexFeeTable {

    /** Enumerates the types of trades. */
    enum class Type {

      /** Active. */
      ACTIVE = 0,

      /** Passive. */
      PASSIVE,

      /** Hidden Active. */
      HIDDEN_ACTIVE,

      /** Hidden Passive. */
      HIDDEN_PASSIVE,

      /** At the open. */
      AT_THE_OPEN,

      /** At the close. */
      AT_THE_CLOSE,

      /** Routed out. */
      ROUTED
    };

    /** The number of trade types enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(7);

    /** The fee table. */
    std::array<Money, TYPE_COUNT> m_feeTable;

    /** The subdollar rates. */
    std::array<boost::rational<int>, TYPE_COUNT> m_subdollarTable;
  };

  /**
   * Parses an AmexFeeTable from a YAML configuration.
   * @param config The configuration to parse the AmexFeeTable from.
   * @return The AmexFeeTable represented by the <i>config</i>.
   */
  inline AmexFeeTable ParseAmexFeeTable(const YAML::Node& config) {
    auto feeTable = AmexFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    ParseFeeTable(config, "subdollar_table",
      Beam::Store(feeTable.m_subdollarTable));
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The AmexFeeTable used to lookup the fee.
   * @param type The trade's type.
   * @return The fee corresponding to the specified <i>type</i>.
   */
  inline Money LookupFee(const AmexFeeTable& feeTable,
      AmexFeeTable::Type type) {
    return feeTable.m_feeTable[static_cast<int>(type)];
  }

  /**
   * Looks up a fee.
   * @param feeTable The AmexFeeTable used to lookup the fee.
   * @param type The trade's type.
   * @return The fee corresponding to the specified <i>type</i>.
   */
  inline boost::rational<int> LookupSubdollarFee(const AmexFeeTable& feeTable,
      AmexFeeTable::Type type) {
    return feeTable.m_subdollarTable[static_cast<int>(type)];
  }

  /**
   * Tests if an OrderFields represents a hidden liquidity provider.
   * @param fields The OrderFields to test.
   * @return <code>true</code> iff the <i>order</i> counts as a hidden liquidity
   *         provider.
   */
  inline bool IsAmexHiddenLiquidityProvider(
      const OrderExecutionService::OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      OrderExecutionService::HasField(fields, Tag{18, "M"});
  }

  /**
   * Calculates the fee on a trade executed on AMEX.
   * @param feeTable The AmexFeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const AmexFeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto type =
      [&] {
        if(executionReport.m_liquidityFlag == "1") {
          if(IsAmexHiddenLiquidityProvider(fields)) {
            return AmexFeeTable::Type::HIDDEN_ACTIVE;
          } else {
            return AmexFeeTable::Type::ACTIVE;
          }
        } else if(executionReport.m_liquidityFlag == "2") {
          if(IsAmexHiddenLiquidityProvider(fields)) {
            return AmexFeeTable::Type::HIDDEN_PASSIVE;
          } else {
            return AmexFeeTable::Type::PASSIVE;
          }
        } else if(executionReport.m_liquidityFlag == "5") {
          return AmexFeeTable::Type::AT_THE_OPEN;
        } else if(executionReport.m_liquidityFlag == "7") {
          return AmexFeeTable::Type::AT_THE_CLOSE;
        } else if(executionReport.m_liquidityFlag.size() > 0 &&
            std::isalpha(executionReport.m_liquidityFlag.front())) {
          return AmexFeeTable::Type::ROUTED;
        } else {
          std::cout << "Unknown liquidity flag [AMEX]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return AmexFeeTable::Type::ACTIVE;
        }
      }();
    if(executionReport.m_lastPrice >= Money::ONE) {
      auto fee = LookupFee(feeTable, type);
      return executionReport.m_lastQuantity * fee;
    } else {
      auto rate = LookupSubdollarFee(feeTable, type);
      return rate *
        (executionReport.m_lastQuantity * executionReport.m_lastPrice);
    }
  }
}

#endif
