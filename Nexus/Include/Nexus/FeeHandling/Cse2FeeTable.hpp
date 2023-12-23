#ifndef NEXUS_CSE2_FEE_TABLE_HPP
#define NEXUS_CSE2_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
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
      m_regularTable;

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
    ParseFeeTable(
      config, "regular_table", Beam::Store(feeTable.m_regularTable));
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
   * Determines what section of the CSE2 fee table is needed to calculate a fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The section within the <i>Cse2FeeTable</i> needed to calculate the
   *         fee for the specified <i>executionReport</i>.
   */
  inline Cse2FeeTable::Section LookupCse2FeeTableSection(
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return Cse2FeeTable::Section::REGULAR;
  }

  /**
   * Calculates a regular fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateRegularFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return Money::ZERO;
  }

  /**
   * Calculates a dark fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateDarkFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return Money::ZERO;
  }

  /**
   * Calculates a debentures or notes fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateDebenturesOrNotesFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return Money::ZERO;
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
    return Money::ZERO;
  }

  /**
   * Calculates an oddlot fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateOddLotFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return Money::ZERO;
  }

  /**
   * Calculates an unintentional cross fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateUnintentionalCrossFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return Money::ZERO;
  }

  /**
   * Calculates an intentional cross fee executed on CSE2.
   * @param feeTable The Cse2FeeTable used to calculate the fee.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money CalculateIntentionalCrossFee(const Cse2FeeTable& feeTable,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return Money::ZERO;
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
    } else if(section == Cse2FeeTable::Section::UNINTENTIONAL_CROSS) {
      return CalculateUnintentionalCrossFee(feeTable, executionReport);
    } else if(section == Cse2FeeTable::Section::INTENTIONAL_CROSS) {
      return CalculateIntentionalCrossFee(feeTable, executionReport);
    }
    std::cout << "Unknown liquidity flag [CSE2]: \"" <<
      executionReport.m_liquidityFlag << "\"\n";
    return CalculateRegularFee(feeTable, executionReport);
  }
}

#endif
