#ifndef NEXUS_LYNXFEETABLE_HPP
#define NEXUS_LYNXFEETABLE_HPP
#include <array>
#include <exception>
#include <unordered_map>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /*! \struct LynxFeeTable
      \brief Stores the table of fees used by the LYNX ATS.
   */
  struct LynxFeeTable {

    /*! \enum LiquidityLevel
        \brief Enumerates the liquidity levels used by LYNX.
     */
    enum class LiquidityLevel : int {

      //! Unknown.
      NONE = -1,

      //! Liquidity level A.
      LEVEL_A = 0,

      //! Liquidity level B.
      LEVEL_B,

      //! Liquidity level C.
      LEVEL_C,

      //! Liquidity level D.
      LEVEL_D
    };

    //! The number of liquidity levels.
    static const std::size_t LIQUIDITY_LEVEL_COUNT = 4;

    /*! \enum AlternativeTrade
        \brief Enumerates the types of alternative trades on LYNX.
     */
    enum class AlternativeTrade : int {

      //! Unknown.
      NONE = -1,

      //! Sub dollar trade.
      SUB_DOLLAR = 0,

      //! Odd lot trade.
      ODD_LOT,
    };

    //! The number of alternative trades enumerated.
    static const std::size_t ALTERNATIVE_TRADE_COUNT = 2;

    //! The table of liquidity level based fees.
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, LIQUIDITY_LEVEL_COUNT>
      m_liquidityFeeTable;

    //! The table of alternative trade fees.
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, ALTERNATIVE_TRADE_COUNT>
      m_alternativeTradeFeeTable;

    //! Maps a Security to its liquidity level.
    std::unordered_map<Security, LiquidityLevel> m_liquidityLevels;
  };

  //! Parses the list of LYNX liquidity levels.
  /*!
    \param config The configuration to parse the liquidity levels from.
    \param marketDatabase The MarketDatabase used to parse Securities.
    \return The map from Security to liquidity levels.
  */
  inline std::unordered_map<Security, LynxFeeTable::LiquidityLevel>
      ParseLynxLiquidityLevels(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    std::unordered_map<Security, LynxFeeTable::LiquidityLevel> liquidityLevels;
    for(auto& item : config) {
      auto symbol = Beam::Extract<std::string>(item, "symbol");
      auto liquidityLevel = Beam::Extract<std::string>(item, "level");
      if(liquidityLevel.size() != 1 ||
          liquidityLevel < "A" || liquidityLevel > "D") {
        BOOST_THROW_EXCEPTION(
          std::runtime_error{"Unrecognized liquidity level found."});
      }
      auto security = ParseSecurity(symbol, marketDatabase);
      auto level = static_cast<LynxFeeTable::LiquidityLevel>(
        int{liquidityLevel[0] - 'A'});
      liquidityLevels.insert(std::make_pair(security, level));
    }
    return liquidityLevels;
  }

  //! Parses a LynxFeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the LynxFeeTable from.
    \param marketDatabase The MarketDatabase used to parse Securities.
    \return The LynxFeeTable represented by the <i>config</i>.
  */
  inline LynxFeeTable ParseLynxFeeTable(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    LynxFeeTable feeTable;
    ParseFeeTable(config, "liquidity_table",
      Beam::Store(feeTable.m_liquidityFeeTable));
    ParseFeeTable(config, "alternative_table",
      Beam::Store(feeTable.m_alternativeTradeFeeTable));
    auto liquidityLevelsPath = Beam::Extract<std::string>(config,
      "liquidity_levels");
    YAML::Node liquidityLevelsConfig;
    Beam::LoadFile(liquidityLevelsPath, Beam::Store(liquidityLevelsConfig));
    auto symbols = liquidityLevelsConfig.FindValue("symbols");
    if(symbols == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"LYNX symbols list not found."});
    }
    feeTable.m_liquidityLevels = ParseLynxLiquidityLevels(*symbols,
      marketDatabase);
    return feeTable;
  }

  //! Returns a Security's LiquidityLevel.
  /*!
    \param feeTable The LynxFeeTable used to lookup the level.
    \param security The Security to find.
    \return The <i>security</i>'s LiquidityLevel with a default value of
            level A if the <i>security</i> is not part of the LynxFeeTable.
  */
  inline LynxFeeTable::LiquidityLevel GetLiquidityLevel(
      const LynxFeeTable& feeTable, const Security& security) {
    auto level = feeTable.m_liquidityLevels.find(security);
    if(level == feeTable.m_liquidityLevels.end()) {
      return LynxFeeTable::LiquidityLevel::NONE;
    }
    return level->second;
  }

  //! Looks up a liquidity level based fee.
  /*!
    \param feeTable The LynxFeeTable used to lookup the fee.
    \param liquidityFlag The liquidity flag to lookup.
    \param level The liquidity level to lookup.
    \return The liquidity level based fee corresponding to the specified
            <i>liquidityFlag</i> and <i>level</i>.
  */
  inline Money LookupFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag, LynxFeeTable::LiquidityLevel level) {
    return feeTable.m_liquidityFeeTable[static_cast<int>(level)][
      static_cast<int>(liquidityFlag)];
  }

  //! Looks up an alternative trade fee.
  /*!
    \param feeTable The LynxFeeTable used to lookup the fee.
    \param liquidityFlag The liquidity flag to lookup.
    \param type The type of trade to lookup.
    \return The alternative trade fee corresponding to the specified
            <i>liquidityFlag</i> and <i>type</i>.
  */
  inline Money LookupFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag, LynxFeeTable::AlternativeTrade type) {
    return feeTable.m_alternativeTradeFeeTable[static_cast<int>(type)][
      static_cast<int>(liquidityFlag)];
  }

  //! Calculates the fee on a trade executed on LYNX.
  /*!
    \param feeTable The LynxFeeTable used to calculate the fee.
    \param security The Security the fee is calculated on.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return The fee calculated for the specified trade.
  */
  inline Money CalculateFee(const LynxFeeTable& feeTable,
      const Security& security, const OrderExecutionService::ExecutionReport&
      executionReport) {
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
    auto alternativeTradeType = LynxFeeTable::AlternativeTrade::NONE;
    auto liquidityLevel = LynxFeeTable::LiquidityLevel::NONE;
    if(executionReport.m_lastQuantity < 100) {
      alternativeTradeType = LynxFeeTable::AlternativeTrade::ODD_LOT;
    } else if(executionReport.m_lastPrice < Money::ONE) {
      alternativeTradeType = LynxFeeTable::AlternativeTrade::SUB_DOLLAR;
    } else {
      liquidityLevel = GetLiquidityLevel(feeTable, security);
      if(liquidityLevel == LynxFeeTable::LiquidityLevel::NONE) {
        if(liquidityFlag == LiquidityFlag::ACTIVE) {
          liquidityLevel = LynxFeeTable::LiquidityLevel::LEVEL_D;
        } else {
          liquidityLevel = LynxFeeTable::LiquidityLevel::LEVEL_A;
        }
      }
    }
    auto fee = [&] {
      if(alternativeTradeType != LynxFeeTable::AlternativeTrade::NONE) {
        return LookupFee(feeTable, liquidityFlag, alternativeTradeType);
      } else {
        return LookupFee(feeTable, liquidityFlag, liquidityLevel);
      }
    }();
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
