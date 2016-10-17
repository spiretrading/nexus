#ifndef NEXUS_NSDQFEETABLE_HPP
#define NEXUS_NSDQFEETABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /*! \struct NsdqFeeTable
      \brief Stores the table of fees used by NSDQ.
   */
  struct NsdqFeeTable {

    /*! \enum Type
        \brief Enumerates the types of trades.
     */
    enum class Type : int {

      //! Unknown.
      NONE = -1,

      //! Active.
      ACTIVE = 0,

      //! Passive.
      PASSIVE,

      //! Passive hidden
      HIDDEN
    };

    //! The number of trade types enumerated.
    static const std::size_t TYPE_COUNT = 3;

    //! The fee table.
    std::array<Money, TYPE_COUNT> m_feeTable;

    //! The sub-dollar rate.
    boost::rational<int> m_subDollarRate;
  };

  //! Parses a NsdqFeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the NsdqFeeTable from.
    \return The NsdqFeeTable represented by the <i>config</i>.
  */
  inline NsdqFeeTable ParseNsdqFeeTable(const YAML::Node& config) {
    NsdqFeeTable feeTable;
    feeTable.m_subDollarRate = Beam::Extract<boost::rational<int>>(config,
      "sub_dollar_rate");
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    return feeTable;
  }

  //! Looks up a fee.
  /*!
    \param feeTable The NsdqFeeTable used to lookup the fee.
    \param type The trade's type.
    \return The fee corresponding to the specified <i>type</i>.
  */
  inline Money LookupFee(const NsdqFeeTable& feeTable,
      NsdqFeeTable::Type type) {
    return feeTable.m_feeTable[static_cast<int>(type)];
  }

  //! Calculates the fee on a trade executed on NSDQ.
  /*!
    \param feeTable The NsdqFeeTable used to calculate the fee.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return The fee calculated for the specified trade.
  */
  inline Money CalculateFee(const NsdqFeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    if(executionReport.m_lastPrice < Money::ONE) {
      return feeTable.m_subDollarRate *
        (executionReport.m_lastQuantity * executionReport.m_lastPrice);
    }
    auto type = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'A' ||
            executionReport.m_liquidityFlag[0] == '7' ||
            executionReport.m_liquidityFlag[0] == '8' ||
            executionReport.m_liquidityFlag[0] == 'a' ||
            executionReport.m_liquidityFlag[0] == 'x' ||
            executionReport.m_liquidityFlag[0] == 'y' ||
            executionReport.m_liquidityFlag[0] == 'e') {
          return NsdqFeeTable::Type::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'k') {
          return NsdqFeeTable::Type::HIDDEN;
        } else if(executionReport.m_liquidityFlag[0] == 'R' ||
            executionReport.m_liquidityFlag[0] == 'm' ||
            executionReport.m_liquidityFlag[0] == 'X') {
          return NsdqFeeTable::Type::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [NSDQ]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return NsdqFeeTable::Type::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [NSDQ]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return NsdqFeeTable::Type::ACTIVE;
      }
    }();
    auto fee = LookupFee(feeTable, type);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
