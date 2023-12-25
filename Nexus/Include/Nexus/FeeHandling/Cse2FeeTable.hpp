#ifndef NEXUS_CSE2_FEE_TABLE_HPP
#define NEXUS_CSE2_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by the CSE2 destination. */
  struct Cse2FeeTable {

    /** Enumerates the sections within the fee table. */
    enum class Section {

      /** Regular fee table. */
      REGULAR,

      /** Dark fee table. */
      DARK,

      /** Debentures or notes table. */
      DEBENTURES_OR_NOTES,

      /** CSE Government listed bond table. */
      CSE_LISTED_GOVERNMENT_BONDS,

      /** Oddlot table. */
      ODDLOT
    };

    /** Enumerates the types of price classes. */
    enum class PriceClass {

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
    std::array<std::array<Money, PRICE_CLASS_COUNT>, LIQUIDITY_FLAG_COUNT>
      m_regularTable;

    /** The dark order fees. */
    std::array<Money, LIQUIDITY_FLAG_COUNT> m_darkTable;

    /** The debutures or notes table. */
    std::array<std::array<Money, LISTING_MARKET_COUNT>, LIQUIDITY_FLAG_COUNT>
      m_debenturesOrNotesTable;

    /** The CSE listed government bonds fees. */
    std::array<Money, LIQUIDITY_FLAG_COUNT> m_cseListedGovernmentBondsTable;

    /** The oddlot fees. */
    std::array<Money, LIQUIDITY_FLAG_COUNT> m_oddlotTable;
  };

  /**
   * Parses a Cse2FeeTable from a YAML configuration.
   * @param config The configuration to parse the CseFeeTable from.
   * @return The Cse2FeeTable represented by the <i>config</i>.
   */
  inline Cse2FeeTable ParseCse2FeeTable(const YAML::Node& config) {
    auto feeTable = Cse2FeeTable();
    ParseFeeTable(
      config, "regular_table", Beam::Store(feeTable.m_regularTable));
    ParseFeeTable(config, "dark_table", Beam::Store(feeTable.m_darkTable));
    ParseFeeTable(config, "debentures_or_notes_table",
      Beam::Store(feeTable.m_debenturesOrNotesTable));
    ParseFeeTable(config, "cse_listed_government_bonds_table",
      Beam::Store(feeTable.m_cseListedGovernmentBondsTable));
    ParseFeeTable(config, "oddlot_table", Beam::Store(feeTable.m_oddlotTable));
    return feeTable;
  }

  /**
   * Determines what section of the CSE2 fee table is needed to calculate a fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The section within the <i>Cse2FeeTable</i> needed to calculate the
   *         fee for the specified <i>executionReport</i>.
   */
  inline Cse2FeeTable::Section LookupCse2FeeTableSection(
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_liquidityFlag.size() >= 3 &&
        executionReport.m_liquidityFlag[2] == 'D') {
      return Cse2FeeTable::Section::DARK;
    } else if(executionReport.m_lastQuantity < 100) {
      return Cse2FeeTable::Section::ODDLOT;
    } else if(fields.m_security.GetSymbol().find(".DB") != std::string::npos ||
        fields.m_security.GetSymbol().find(".NT") != std::string::npos ||
        fields.m_security.GetSymbol().find(".NO") != std::string::npos ||
        fields.m_security.GetSymbol().find(".NS") != std::string::npos) {
      return Cse2FeeTable::Section::DEBENTURES_OR_NOTES;
    }
    return Cse2FeeTable::Section::REGULAR;
  }

  /**
   * Returns the liquidity flag for a CSE2 execution report.
   * @param executionReport An ExecutionReport received from CSE2.
   * @return The <i>executionReport</i>'s liquidity flag.
   */
  inline LiquidityFlag LookupCse2LiquidityFlag(
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_liquidityFlag.empty() ||
        executionReport.m_liquidityFlag[0] == 'P') {
      return LiquidityFlag::PASSIVE;
    }
    return LiquidityFlag::ACTIVE;
  }

  /**
   * Calculates a regular fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateRegularFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto liquidityFlag = LookupCse2LiquidityFlag(executionReport);
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < Money::ONE) {
        return Cse2FeeTable::PriceClass::SUBDOLLAR;
      }
      return Cse2FeeTable::PriceClass::DEFAULT;
    }();
    auto fee = feeTable.m_regularTable[static_cast<int>(liquidityFlag)][
      static_cast<int>(priceClass)];
    return executionReport.m_lastQuantity * fee;
  }

  /**
   * Calculates a dark fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateDarkFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto liquidityFlag = LookupCse2LiquidityFlag(executionReport);
    auto fee = feeTable.m_darkTable[static_cast<int>(liquidityFlag)];
    return executionReport.m_lastQuantity * fee;
  }

  /**
   * Calculates a debentures or notes fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateDebenturesOrNotesFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto liquidityFlag = LookupCse2LiquidityFlag(executionReport);
    auto listingMarket = [&] {
      if(executionReport.m_liquidityFlag.size() <= 1 ||
          executionReport.m_liquidityFlag[1] == 'T' ||
          executionReport.m_liquidityFlag[1] == 'V' ||
          executionReport.m_liquidityFlag[1] == 'N') {
        return Cse2FeeTable::ListingMarket::TSX_TSXV;
      }
      return Cse2FeeTable::ListingMarket::CSE;
    }();
    auto fee = feeTable.m_debenturesOrNotesTable[
      static_cast<int>(liquidityFlag)][static_cast<int>(listingMarket)];
    return executionReport.m_lastQuantity * fee;
  }

  /**
   * Calculates a CSE listed government bond fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateCseListedGovernmentBondsFee(
      const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto liquidityFlag = LookupCse2LiquidityFlag(executionReport);
    auto fee = feeTable.m_cseListedGovernmentBondsTable[
      static_cast<int>(liquidityFlag)];
    return executionReport.m_lastQuantity * fee;
  }

  /**
   * Calculates an oddlot fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateOddLotFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto liquidityFlag = LookupCse2LiquidityFlag(executionReport);
    auto fee = feeTable.m_oddlotTable[static_cast<int>(liquidityFlag)];
    return executionReport.m_lastQuantity * fee;
  }

  /**
   * Calculates the fee on a trade executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto section = LookupCse2FeeTableSection(fields, executionReport);
    if(section == Cse2FeeTable::Section::REGULAR) {
      return CalculateRegularFee(feeTable, executionReport);
    } else if(section == Cse2FeeTable::Section::DARK) {
      return CalculateDarkFee(feeTable, executionReport);
    } else if(section == Cse2FeeTable::Section::DEBENTURES_OR_NOTES) {
      return CalculateDebenturesOrNotesFee(feeTable, executionReport);
    } else if(section == Cse2FeeTable::Section::CSE_LISTED_GOVERNMENT_BONDS) {
      return CalculateCseListedGovernmentBondsFee(feeTable, executionReport);
    } else if(section == Cse2FeeTable::Section::ODDLOT) {
      return CalculateOddLotFee(feeTable, executionReport);
    }
    std::cout << "Unknown liquidity flag [CSE2]: \"" <<
      executionReport.m_liquidityFlag << "\"\n";
    return CalculateRegularFee(feeTable, executionReport);
  }
}

#endif
