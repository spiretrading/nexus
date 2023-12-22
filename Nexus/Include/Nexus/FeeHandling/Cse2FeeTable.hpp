#ifndef NEXUS_CSE2_FEE_TABLE_HPP
#define NEXUS_CSE2_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {

  /** Stores the table of fees used by the CSE2 destination. */
  struct Cse2FeeTable {

    /** Enumerates the types of fee tables. */
    enum class FeeTable {

      /** Regular fee table. */
      REGULAR,

      /** Dark fee table. */
      DARK,

      /** Debentures or notes table. */
      DEBENTURES_OR_NOTES,

      /** CSE Government listed bond table. */
      CSE_GOVERNMENT_LISTED_BONDS,

      /** Oddlot table. */
      ODDLOT,

      /** Unintentional cross. */
      UNINTENTIONAL_CROSS,

      /** Intentional cross. */
      INTENTIONAL_CROSS
    };

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price >= $1.00. */
      DEFAULT,

      /** Price >= $0.10 & < $1.00. */
      SUBDOLLAR
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(2);

    /** The market the security is listed on. */
    enum class ListingMarket {

      /** CSE listed. */
      CSE,

      /** TSX or TSX-V listed. */
      TSX_TSXV
    };

    /** The number of listings. */
    static constexpr auto LISTING_MARKET_COUNT = std::size_t(2);

    /** The regular fee table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_feeTable;

    /** The dark order fees. */
    std::array<Money, LIQUIDITY_FLAG_COUNT> m_darkTable;

    /** The debutures or notes table. */
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, LISTING_MARKET_COUNT>
      m_debenturesOrNotesTable;

    /** The CSE listed government bonds fees. */
    std::array<Money, LIQUIDITY_FLAG_COUNT> m_cseListedGovernmentBondsTable;

    /** The oddlot fees. */
    std::array<Money, LIQUIDITY_FLAG_COUNT> m_oddlotTable;

    /** The unintentional cross fees. */
    std::array<Money, LIQUIDITY_FLAG_COUNT> m_unintentionalCrossTable;

    /** The intentional cross credit. */
    Money m_intentionalCrossCredit;
  };

  /**
   * Parses a Cse2FeeTable from a YAML configuration.
   * @param config The configuration to parse the CseFeeTable from.
   * @return The Cse2FeeTable represented by the <i>config</i>.
   */
  inline Cse2FeeTable ParseCse2FeeTable(const YAML::Node& config) {
    auto feeTable = Cse2FeeTable();
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    ParseFeeTable(config, "dark_table", Beam::Store(feeTable.m_darkTable));
    ParseFeeTable(config, "debentures_or_notes_table",
      Beam::Store(feeTable.m_debenturesOrNotesTable));
    ParseFeeTable(config, "cse_listed_government_bonds_table",
      Beam::Store(feeTable.m_cseListedGovernmentBondsTable));
    ParseFeeTable(config, "oddlot_table", Beam::Store(feeTable.m_oddlotTable));
    ParseFeeTable(config, "unintentional_cross_table",
      Beam::Store(feeTable.m_unintentionalCrossTable));
    feeTable.m_intentionalCrossCredit =
      Beam::Extract<Money>(config, "intentional_cross_credit");
    return feeTable;
  }

  /**
   * Looks up a fee.
   * @param feeTable The Cse2FeeTable used to lookup the fee.
   * @param liquidityFlag The trade's LiquidityFlag.
   * @param priceClass The trade's PriceClass.
   * @return The fee corresponding to the specified <i>liquidityFlag</i> and
   *         <i>priceClass</i>.
   */
  inline Money LookupFee(const Cse2FeeTable& feeTable,
      LiquidityFlag liquidityFlag, CseFeeTable::PriceClass priceClass) {
    return feeTable.m_feeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  /**
   * Calculates the fee on a trade executed on CSE.
   * @param feeTable The CseFeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const CseFeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < 10 * Money::CENT) {
        return CseFeeTable::PriceClass::SUBDIME;
      } else if(executionReport.m_lastPrice < Money::ONE) {
        return CseFeeTable::PriceClass::SUBDOLLAR;
      } else {
        return CseFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto liquidityFlag = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P') {
          return LiquidityFlag::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'A') {
          return LiquidityFlag::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [CSE]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return LiquidityFlag::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [CSE]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    }();
    auto fee = LookupFee(feeTable, liquidityFlag, priceClass);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
