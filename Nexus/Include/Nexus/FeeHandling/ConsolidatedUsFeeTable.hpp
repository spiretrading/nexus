#ifndef NEXUS_CONSOLIDATED_US_FEE_TABLE_HPP
#define NEXUS_CONSOLIDATED_US_FEE_TABLE_HPP
#include <exception>
#include <sstream>
#include <unordered_set>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/AmexFeeTable.hpp"
#include "Nexus/FeeHandling/ArcaFeeTable.hpp"
#include "Nexus/FeeHandling/BatsFeeTable.hpp"
#include "Nexus/FeeHandling/BatyFeeTable.hpp"
#include "Nexus/FeeHandling/EdgaFeeTable.hpp"
#include "Nexus/FeeHandling/EdgxFeeTable.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/NsdqFeeTable.hpp"
#include "Nexus/FeeHandling/NyseFeeTable.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Consolidates all U.S. related market fees together. */
  struct ConsolidatedUsFeeTable {

    /** Fee charged for the software. */
    Money m_spireFee;

    /** The SEC rate. */
    boost::rational<int> m_secRate;

    /** The TAF fee. */
    Money m_tafFee;

    /** The NSCC rate. */
    boost::rational<int> m_nsccRate;

    /** The clearing fee. */
    Money m_clearingFee;

    /** Fee table used by AMEX. */
    AmexFeeTable m_amexFeeTable;

    /** Fee table used by ARCA. */
    ArcaFeeTable m_arcaFeeTable;

    /** Fee table used by BATS. */
    BatsFeeTable m_batsFeeTable;

    /** Fee table used by BATY. */
    BatyFeeTable m_batyFeeTable;

    /** Fee table used by EDGA. */
    EdgaFeeTable m_edgaFeeTable;

    /** Fee table used by EDGX. */
    EdgxFeeTable m_edgxFeeTable;

    /** Fee table used by NSDQ. */
    NsdqFeeTable m_nsdqFeeTable;

    /** Fee table used by NYSE. */
    NyseFeeTable m_nyseFeeTable;
  };

  /**
   * Parses a ConsolidatedUsFeeTable from a YAML configuration.
   * @param config The configuration to parse the ConsolidatedUsFeeTable from.
   * @param marketDatabase The MarketDatabase used to parse Securities.
   * @return The ConsolidatedUsFeeTable represented by the <i>config</i>.
   */
  inline ConsolidatedUsFeeTable ParseConsolidatedUsFeeTable(
      const YAML::Node& config, const MarketDatabase& marketDatabase) {
    auto feeTable = ConsolidatedUsFeeTable();
    feeTable.m_spireFee = Beam::Extract<Money>(config, "spire_fee");
    feeTable.m_secRate = Beam::Extract<boost::rational<int>>(
      config, "sec_rate");
    feeTable.m_tafFee = Beam::Extract<Money>(config, "taf_fee");
    feeTable.m_nsccRate = Beam::Extract<boost::rational<int>>(
      config, "nscc_rate");
    feeTable.m_clearingFee = Beam::Extract<Money>(config, "clearing_fee");
    auto amexConfig = config["amex"];
    if(!amexConfig) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for AMEX missing."));
    } else {
      feeTable.m_amexFeeTable = ParseAmexFeeTable(amexConfig);
    }
    auto arcaConfig = config["arca"];
    if(!arcaConfig) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for ARCA missing."));
    } else {
      feeTable.m_arcaFeeTable = ParseArcaFeeTable(arcaConfig);
    }
    auto batsConfig = config["bats"];
    if(!batsConfig) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for BATS missing."));
    } else {
      feeTable.m_batsFeeTable = ParseBatsFeeTable(batsConfig);
    }
    auto batyConfig = config["baty"];
    if(!batyConfig) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for BATY missing."));
    } else {
      feeTable.m_batyFeeTable = ParseBatyFeeTable(batyConfig);
    }
    auto edgaConfig = config["edga"];
    if(!edgaConfig) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for EDGA missing."));
    } else {
      feeTable.m_edgaFeeTable = ParseEdgaFeeTable(edgaConfig);
    }
    auto edgxConfig = config["edgx"];
    if(!edgxConfig) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for EDGX missing."));
    } else {
      feeTable.m_edgxFeeTable = ParseEdgxFeeTable(edgxConfig);
    }
    auto nasdaqConfig = config["nasdaq"];
    if(!nasdaqConfig) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error("Fee table for NASDAQ missing."));
    } else {
      feeTable.m_nsdqFeeTable = ParseNsdqFeeTable(nasdaqConfig);
    }
    auto nyseConfig = config["nyse"];
    if(!nyseConfig) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for NYSE missing."));
    } else {
      feeTable.m_nyseFeeTable = ParseNyseFeeTable(nyseConfig);
    }
    return feeTable;
  }

  /**
   * Calculates the fee on a trade executed on a U.S. market.
   * @param feeTable The ConsolidatedUsFeeTable used to calculate the fee.
   * @param order The Order that was traded against.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return An ExecutionReport containing the calculated fees.
   */
  inline OrderExecutionService::ExecutionReport CalculateFee(
      const ConsolidatedUsFeeTable& feeTable,
      const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = executionReport;
    auto lastMarket = [&] {
      auto& destination = order.GetInfo().m_fields.m_destination;
      if(destination == DefaultDestinations::AMEX()) {
        return boost::lexical_cast<std::string>(DefaultMarkets::ASEX());
      } else if(destination == DefaultDestinations::ARCA()) {
        return boost::lexical_cast<std::string>(DefaultMarkets::ARCX());
      } else if(destination == DefaultDestinations::BATS()) {
        return boost::lexical_cast<std::string>(DefaultMarkets::BATS());
      } else if(destination == DefaultDestinations::BATY()) {
        return boost::lexical_cast<std::string>(DefaultMarkets::BATY());
      } else if(destination == DefaultDestinations::EDGA()) {
        return boost::lexical_cast<std::string>(DefaultMarkets::EDGA());
      } else if(destination == DefaultDestinations::EDGX()) {
        return boost::lexical_cast<std::string>(DefaultMarkets::EDGX());
      } else if(destination == DefaultDestinations::NASDAQ()) {
        return boost::lexical_cast<std::string>(DefaultMarkets::NASDAQ());
      } else if(destination == DefaultDestinations::NYSE()) {
        return boost::lexical_cast<std::string>(DefaultMarkets::NYSE());
      } else {
        return std::string();
      }
    }();
    feesReport.m_executionFee += [&] {
      if(lastMarket == DefaultMarkets::ASEX()) {
        return CalculateFee(feeTable.m_amexFeeTable, order.GetInfo().m_fields,
          executionReport);
      } else if(lastMarket == DefaultMarkets::ARCX()) {
        return CalculateFee(feeTable.m_arcaFeeTable, order.GetInfo().m_fields,
          executionReport);
      } else if(lastMarket == DefaultMarkets::BATS()) {
        return CalculateFee(feeTable.m_batsFeeTable, executionReport);
      } else if(lastMarket == DefaultMarkets::BATY()) {
        return CalculateFee(feeTable.m_batyFeeTable, executionReport);
      } else if(lastMarket == DefaultMarkets::EDGA()) {
        return CalculateFee(feeTable.m_edgaFeeTable, executionReport);
      } else if(lastMarket == DefaultMarkets::EDGX()) {
        return CalculateFee(feeTable.m_edgxFeeTable, executionReport);
      } else if(lastMarket == DefaultMarkets::NASDAQ()) {
        return CalculateFee(feeTable.m_nsdqFeeTable, executionReport);
      } else if(lastMarket == DefaultMarkets::NYSE()) {
        return CalculateFee(feeTable.m_nyseFeeTable, order.GetInfo().m_fields,
          executionReport);
      } else {
        std::cout << "Unknown last market [US]: \"" <<
          order.GetInfo().m_fields.m_destination << "\"\n";
        return Money::ZERO;
      }
    }();
    feesReport.m_processingFee += [&] {
      if(feesReport.m_lastQuantity != 0) {
        auto processingFee = feesReport.m_lastQuantity *
          (feeTable.m_clearingFee + feeTable.m_tafFee);
        if(order.GetInfo().m_fields.m_side == Side::BID) {
          processingFee += feeTable.m_secRate *
            (feesReport.m_lastQuantity * feesReport.m_lastPrice);
        }
        processingFee += Money::CENT + feeTable.m_nsccRate *
          (feesReport.m_lastQuantity * feesReport.m_lastPrice);
        return Ceil(processingFee, 3);
      } else {
        return Money::ZERO;
      }
    }();
    feesReport.m_commission += feesReport.m_lastQuantity * feeTable.m_spireFee;
    return feesReport;
  }
}

#endif
