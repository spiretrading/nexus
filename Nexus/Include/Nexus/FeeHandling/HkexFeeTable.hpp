#ifndef NEXUS_HKEX_FEE_TABLE_HPP
#define NEXUS_HKEX_FEE_TABLE_HPP
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by HKEX. */
  struct HkexFeeTable {

    /** Fee charged for the software. */
    Money m_spireFee;

    /** The STAMP tax. */
    boost::rational<int> m_stampTax;

    /** The levy fee. */
    boost::rational<int> m_levy;

    /** The trading fee. */
    boost::rational<int> m_tradingFee;

    /** The brokerage fee. */
    boost::rational<int> m_brokerageFee;

    /** The CCASS fee. */
    boost::rational<int> m_ccassFee;

    /** The minimum CCASS fee per trade. */
    Money m_minimumCcassFee;

    /** The list of securities that are charged the STAMP tax. */
    std::unordered_set<Security> m_stampApplicability;
  };

  /**
   * Parses the set of HKEX symbols subject to the STAMP tax.
   * @param path The path to the YAML file to parse.
   * @param marketDatabase The MarketDatabase used to parse the symbols.
   * @return The set of symbols subject to the STAMP tax.
   */
  inline std::unordered_set<Security> ParseHkexStampSecurities(
      const std::string& path, const MarketDatabase& marketDatabase) {
    auto config = Beam::LoadFile(path);
    auto symbols = config["symbols"];
    if(!symbols) {
      BOOST_THROW_EXCEPTION(std::runtime_error(
        "Interlisted symbols not found."));
    }
    return ParseSecuritySet(symbols, marketDatabase);
  }

  /**
   * Parses an HkexFeeTable from a YAML configuration.
   * @param config The configuration to parse the HkexFeeTable from.
   * @param marketDatabase The MarketDatabase used to parse Securities.
   * @return The HkexFeeTable represented by the <i>config</i>.
   */
  inline HkexFeeTable ParseHkexFeeTable(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    auto feeTable = HkexFeeTable();
    feeTable.m_spireFee = Beam::Extract<Money>(config, "spire_fee");
    feeTable.m_stampTax = Beam::Extract<boost::rational<int>>(config,
      "stamp_tax");
    feeTable.m_levy = Beam::Extract<boost::rational<int>>(config, "levy");
    feeTable.m_tradingFee = Beam::Extract<boost::rational<int>>(config,
      "trading_fee");
    feeTable.m_brokerageFee = Beam::Extract<boost::rational<int>>(config,
      "brokerage_fee");
    feeTable.m_ccassFee = Beam::Extract<boost::rational<int>>(config,
      "ccass_fee");
    feeTable.m_minimumCcassFee = Beam::Extract<Money>(config,
      "minimum_ccass_fee");
    auto stampPath = Beam::Extract<std::string>(config, "stamp_path");
    feeTable.m_stampApplicability = ParseHkexStampSecurities(stampPath,
      marketDatabase);
    return feeTable;
  }

  /**
   * Calculates the fee on a trade executed on HKEX.
   * @param feeTable The HkexFeeTable used to calculate the fee.
   * @param fields The OrderFields used to place the Order.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return An ExecutionReport containing the calculated fees.
   */
  inline OrderExecutionService::ExecutionReport CalculateFee(
      const HkexFeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return executionReport;
    }
    auto feesReport = executionReport;
    feesReport.m_commission += feeTable.m_spireFee;
    auto notionalValue = feesReport.m_lastQuantity * feesReport.m_lastPrice;
    feesReport.m_processingFee += feeTable.m_brokerageFee * notionalValue;
    feesReport.m_executionFee += [&] {
      auto tradeFee = feeTable.m_levy * notionalValue;
      tradeFee += feeTable.m_tradingFee * notionalValue;
      auto ccassFee = feeTable.m_ccassFee * notionalValue;
      tradeFee += std::max(ccassFee, feeTable.m_minimumCcassFee);
      if(feeTable.m_stampApplicability.find(fields.m_security) !=
          feeTable.m_stampApplicability.end()) {
        tradeFee += feeTable.m_stampTax * notionalValue;
      }
      return tradeFee;
    }();
    return feesReport;
  }
}

#endif
