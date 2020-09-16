#ifndef NEXUS_XATS_FEE_TABLE_HPP
#define NEXUS_XATS_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the table of fees used by Alpha on TSX. */
  struct XatsFeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price < $0.50. */
      SUBHALF_DOLLAR = 0,

      /** Price >= $0.50 & < $1.00. */
      SUBDOLLAR,

      /** Price >= $1.00 & < $5.00. */
      SUBFIVE_DOLLAR,

      /** Price >= $5.00. */
      DEFAULT,
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(4);

    /** Enumerates the types of trades. */
    enum class Type {

      /** Unknown. */
      NONE = -1,

      /** Continuous - Active, including odd-lots. */
      ACTIVE = 0,

      /** Continuous - Passive. */
      PASSIVE,

      /** Auction Opening */
      OPEN_AUCTION,

      /** IntraSpread SDL-to-Dark Active */
      SDL_ACTIVE,

      /** IntraSpread SDL-to-Dark Passive */
      SDL_PASSIVE,

      /** IntraSpread Dark-to-Active */
      INTRASPREAD_ACTIVE,

      /** IntraSpread Dark-to-Passive */
      INTRASPREAD_PASSIVE
    };

    /** The number of trade types enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(7);

    /** The general fee table. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT>
      m_generalFeeTable;

    /** The ETF fee table. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT> m_etfFeeTable;

    /**
     * The max fee charged on IntraSpread Dark executions that are below a
     * dollar.
     */
    Money m_intraspreadDarkToDarkSubdollarMaxFee;

    /** The max fee charged on IntraSpread Dark executions. */
    Money m_intraspreadDarkToDarkMaxFee;
  };

  /**
   * Parses an XatsFeeTable from a YAML configuration.
   * @param config The configuration to parse the XatsFeeTable from.
   * @return The XatsFeeTable represented by the <i>config</i>.
   */
  inline XatsFeeTable ParseXatsFeeTable(const YAML::Node& config) {
    auto feeTable = XatsFeeTable();
    ParseFeeTable(config, "general_table",
      Beam::Store(feeTable.m_generalFeeTable));
    ParseFeeTable(config, "etf_table", Beam::Store(feeTable.m_etfFeeTable));
    feeTable.m_intraspreadDarkToDarkSubdollarMaxFee =
      Beam::Extract<Money>(config, "intraspread_dark_to_dark_subdollar_max");
    feeTable.m_intraspreadDarkToDarkMaxFee = Beam::Extract<Money>(config,
      "intraspread_dark_to_dark_max");
    return feeTable;
  }

  /**
   * Looks up a general fee.
   * @param feeTable The XatsFeeTable used to lookup the fee.
   * @param type The trade's Type.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>type</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupGeneralFee(const XatsFeeTable& feeTable,
      XatsFeeTable::Type type, XatsFeeTable::PriceClass priceClass) {
    return feeTable.m_generalFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(type)];
  }

  /**
   * Looks up an ETF fee.
   * @param feeTable The XatsFeeTable used to lookup the fee.
   * @param type The trade's Type.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>type</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupEtfFee(const XatsFeeTable& feeTable,
      XatsFeeTable::Type type, XatsFeeTable::PriceClass priceClass) {
    return feeTable.m_etfFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(type)];
  }

  /**
   * Calculates the fee on a trade executed on XATS.
   * @param feeTable The XatsFeeTable used to calculate the fee.
   * @param isEtf Whether the calculation is for an ETF.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const XatsFeeTable& feeTable, bool isEtf,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < 50 * Money::CENT) {
        return XatsFeeTable::PriceClass::SUBHALF_DOLLAR;
      } else if(executionReport.m_lastPrice < Money::ONE) {
        return XatsFeeTable::PriceClass::SUBDOLLAR;
      } else if(executionReport.m_lastPrice < 5 * Money::ONE) {
        return XatsFeeTable::PriceClass::SUBFIVE_DOLLAR;
      } else {
        return XatsFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto type = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P') {
          return XatsFeeTable::Type::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'A') {
          return XatsFeeTable::Type::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [XATS]: " <<
            executionReport.m_liquidityFlag << "\n";
          return XatsFeeTable::Type::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [XATS]: " <<
          executionReport.m_liquidityFlag << "\n";
        return XatsFeeTable::Type::ACTIVE;
      }
    }();
    auto fee = [&] {
      if(isEtf) {
        return LookupEtfFee(feeTable, type, priceClass);
      } else {
        return LookupGeneralFee(feeTable, type, priceClass);
      }
    }();
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
