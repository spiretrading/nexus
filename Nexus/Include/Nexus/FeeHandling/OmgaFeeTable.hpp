#ifndef NEXUS_OMGA_FEE_TABLE_HPP
#define NEXUS_OMGA_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by Omega on TSX. */
  struct OmgaFeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price >= $1.00. */
      DEFAULT = 0,

      /** Price < $1.00. */
      SUBDOLLAR,
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(2);

    /** Enumerates the types of trades. */
    enum class Type {

      /** Unknown. */
      NONE = -1,

      /** Passive. */
      PASSIVE = 0,

      /** Active. */
      ACTIVE,

      /** Hidden Passive. */
      HIDDEN_PASSIVE,

      /** Hidden Active. */
      HIDDEN_ACTIVE,

      /** ETF Passive. */
      ETF_PASSIVE,

      /** ETF Active. */
      ETF_ACTIVE,

      /** Odd Lot. */
      ODD_LOT
    };

    /** The number of trade types enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(7);

    /** The fee table. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT> m_feeTable;
  };

  /**
   * Parses an OmgaFeeTable from a YAML configuration.
   * @param config The configuration to parse the OmgaFeeTable from.
   * @return The OmgaFeeTable represented by the <i>config</i>.
   */
  inline OmgaFeeTable ParseOmgaFeeTable(const YAML::Node& config) {
    auto feeTable = OmgaFeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The OmgaFeeTable used to lookup the fee.
   * @param type The trade's Type.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>type</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupFee(const OmgaFeeTable& feeTable, OmgaFeeTable::Type type,
      OmgaFeeTable::PriceClass priceClass) {
    return feeTable.m_feeTable[static_cast<int>(priceClass)][
      static_cast<int>(type)];
  }

  /**
   * Tests if an OrderFields represents a hidden liquidity provider.
   * @param fields The OrderFields to test.
   * @return <code>true</code> iff the <i>order</i> counts as a hidden liquidity
   *         provider.
   */
  inline bool IsOmgaHiddenLiquidityProvider(
      const OrderExecutionService::OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      OrderExecutionService::HasField(fields, Tag{18, "M"});
  }

  /**
   * Calculates the fee on a trade executed on OMGA.
   * @param feeTable The OmgaFeeTable used to calculate the fee.
   * @param isEtf Whether the calculation is for an ETF.
   * @param order The Order the trade took place on.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const OmgaFeeTable& feeTable, bool isEtf,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < Money::ONE) {
        return OmgaFeeTable::PriceClass::SUBDOLLAR;
      } else {
        return OmgaFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto type = [&] {
      if(executionReport.m_lastQuantity < 100) {
        return OmgaFeeTable::Type::ODD_LOT;
      } else if(IsOmgaHiddenLiquidityProvider(fields)) {
        if(executionReport.m_liquidityFlag.size() == 1) {
          if(executionReport.m_liquidityFlag[0] == 'A') {
            return OmgaFeeTable::Type::HIDDEN_ACTIVE;
          } else if(executionReport.m_liquidityFlag[0] == 'P') {
            return OmgaFeeTable::Type::HIDDEN_PASSIVE;
          } else {
            std::cout << "Unknown liquidity flag [OMGA]: \"" <<
              executionReport.m_liquidityFlag << "\"\n";
            return OmgaFeeTable::Type::HIDDEN_PASSIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [OMGA]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return OmgaFeeTable::Type::HIDDEN_PASSIVE;
        }
      } else if(isEtf) {
        if(executionReport.m_liquidityFlag.size() == 1) {
          if(executionReport.m_liquidityFlag[0] == 'A') {
            return OmgaFeeTable::Type::ETF_ACTIVE;
          } else if(executionReport.m_liquidityFlag[0] == 'P') {
            return OmgaFeeTable::Type::ETF_PASSIVE;
          } else {
            std::cout << "Unknown liquidity flag [OMGA]: \"" <<
              executionReport.m_liquidityFlag << "\"\n";
            return OmgaFeeTable::Type::ETF_PASSIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [OMGA]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return OmgaFeeTable::Type::ETF_PASSIVE;
        }
      } else {
        if(executionReport.m_liquidityFlag.size() == 1) {
          if(executionReport.m_liquidityFlag[0] == 'A') {
            return OmgaFeeTable::Type::ACTIVE;
          } else if(executionReport.m_liquidityFlag[0] == 'P') {
            return OmgaFeeTable::Type::PASSIVE;
          } else {
            std::cout << "Unknown liquidity flag [OMGA]: \"" <<
              executionReport.m_liquidityFlag << "\"\n";
            return OmgaFeeTable::Type::PASSIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [OMGA]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return OmgaFeeTable::Type::PASSIVE;
        }
      }
    }();
    auto fee = LookupFee(feeTable, type, priceClass);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
