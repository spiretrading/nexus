#ifndef NEXUS_CHIC_FEE_TABLE_HPP
#define NEXUS_CHIC_FEE_TABLE_HPP
#include <array>
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by CHI-X Canada. */
  struct ChicFeeTable {

    /** Lists the available trade classifications. */
    enum class Classification {

      /** Unknown. */
      NONE = -1,

      /** Interlisted securities. */
      INTERLISTED,

      /** Non-interlisted securities. */
      NON_INTERLISTED,

      /** ETFs ($1.00 or over). */
      ETF,

      /** Securities >= $0.10 and < $1.00 (including interlisted). */
      SUBDOLLAR,

      /** Securities < $0.10 (including interlisted). */
      SUBDIME
    };

    /** The number of classifications enumerated. */
    static constexpr auto CLASSIFICATION_COUNT = std::size_t(5);

    /** Lists the index into a security fee table. */
    enum class Index {

      /** Unknown. */
      NONE = -1,

      /** Liquidity removing. */
      ACTIVE,

      /** Liquidity providing. */
      PASSIVE,

      /** Hidden liquidity active. */
      HIDDEN_ACTIVE,

      /** Hidden liquidity providing. */
      HIDDEN_PASSIVE
    };

    /** The number of indexes enumerated. */
    static constexpr auto INDEX_COUNT = std::size_t(4);

    /** The fee table used for securities. */
    std::array<std::array<Money, INDEX_COUNT>, CLASSIFICATION_COUNT>
      m_securityTable;

    /** The set of interlisted securities. */
    std::unordered_set<Security> m_interlisted;

    /** The set of ETFs. */
    std::unordered_set<Security> m_etfs;
  };

  /**
   * Parses a ChicFeeTable from a YAML configuration.
   * @param config The configuration to parse the ChicFeeTable from.
   * @param etfs The set of ETF Securities.
   * @param interlisted The set of interlisted Securities.
   * @return The ChicFeeTable represented by the <i>config</i>.
   */
  inline ChicFeeTable ParseChicFeeTable(const YAML::Node& config,
      std::unordered_set<Security> etfs,
      std::unordered_set<Security> interlisted) {
    auto feeTable = ChicFeeTable();
    feeTable.m_etfs = std::move(etfs);
    feeTable.m_interlisted = std::move(interlisted);
    ParseFeeTable(config, "security_table",
      Beam::Store(feeTable.m_securityTable));
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The ChicFeeTable used to lookup the fee.
   * @param classification The trade's classification.
   * @param index The index into the fee table.
   * @return The fee corresponding to the specified <i>classification</i> and
   *         <i>index</i>.
   */
  inline Money LookupFee(const ChicFeeTable& feeTable,
      ChicFeeTable::Index index, ChicFeeTable::Classification classification) {
    return feeTable.m_securityTable[static_cast<int>(classification)][
      static_cast<int>(index)];
  }

  /**
   * Calculates the fee on a trade executed on CHIC.
   * @param feeTable The ChicFeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const ChicFeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto classification = [&] {
      if(executionReport.m_lastPrice < 10 * Money::CENT) {
        return ChicFeeTable::Classification::SUBDIME;
      } else if(executionReport.m_lastPrice < Money::ONE) {
        return ChicFeeTable::Classification::SUBDOLLAR;
      } else if(feeTable.m_interlisted.count(fields.m_security) == 1) {
        return ChicFeeTable::Classification::INTERLISTED;
      } else if(feeTable.m_etfs.count(fields.m_security) == 1) {
        return ChicFeeTable::Classification::ETF;
      } else {
        return ChicFeeTable::Classification::NON_INTERLISTED;
      }
    }();
    auto index = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P' ||
            executionReport.m_liquidityFlag[0] == 'S') {
          return ChicFeeTable::Index::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'A' ||
            executionReport.m_liquidityFlag[0] == 'C') {
          return ChicFeeTable::Index::ACTIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'a' ||
            executionReport.m_liquidityFlag[0] == 'd') {
          return ChicFeeTable::Index::HIDDEN_PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'r' ||
            executionReport.m_liquidityFlag[0] == 'D') {
          return ChicFeeTable::Index::HIDDEN_ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [CHIC]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return ChicFeeTable::Index::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [CHIC]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return ChicFeeTable::Index::ACTIVE;
      }
    }();
    auto fee = LookupFee(feeTable, index, classification);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
