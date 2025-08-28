#ifndef NEXUS_CONSOLIDATED_TMX_FEE_TABLE_HPP
#define NEXUS_CONSOLIDATED_TMX_FEE_TABLE_HPP
#include <exception>
#include <sstream>
#include <unordered_set>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandling/Cse2FeeTable.hpp"
#include "Nexus/FeeHandling/CxdFeeTable.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandling/MatnFeeTable.hpp"
#include "Nexus/FeeHandling/NeoeFeeTable.hpp"
#include "Nexus/FeeHandling/NexFeeTable.hpp"
#include "Nexus/FeeHandling/OmgaFeeTable.hpp"
#include "Nexus/FeeHandling/PureFeeTable.hpp"
#include "Nexus/FeeHandling/TsxFeeTable.hpp"
#include "Nexus/FeeHandling/XatsFeeTable.hpp"
#include "Nexus/FeeHandling/Xcx2FeeTable.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Consolidates all TMX related market fees together. */
  struct ConsolidatedTmxFeeTable {

    /** Stores the historical state needed to carry out fee calculations. */
    struct State {

      /** Stores the fees charged per Order. */
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId, Money>
        m_perOrderCharges;

      /** The number of times an Order has been filled. */
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId, int>
        m_fillCount;
    };

    /** The fee charged by the software. */
    Money m_spireFee;

    /** The fee charged by IIROC. */
    Money m_iirocFee;

    /** The CDS fee. */
    Money m_cdsFee;

    /** The CDS cap. */
    int m_cdsCap;

    /** The clearing fee; */
    Money m_clearingFee;

    /** The fee charged per Order traded. */
    Money m_perOrderFee;

    /** The cap charged per Order. */
    Money m_perOrderCap;

    /** Fee table used by CHIC. */
    ChicFeeTable m_chicFeeTable;

    /** Fee table used by CSE. */
    CseFeeTable m_cseFeeTable;

    /** Fee table used by CSE2. */
    Cse2FeeTable m_cse2FeeTable;

    /** Fee table used by CXD. */
    CxdFeeTable m_cxdFeeTable;

    /** Fee table used by LYNX. */
    LynxFeeTable m_lynxFeeTable;

    /** Fee table used by MATN. */
    MatnFeeTable m_matnFeeTable;

    /** Fee table used by NEOE. */
    NeoeFeeTable m_neoeFeeTable;

    /** Fee table used by NEX. */
    NexFeeTable m_nexFeeTable;

    /** Fee table used by OMGA. */
    OmgaFeeTable m_omgaFeeTable;

    /** Fee table used by PURE. */
    PureFeeTable m_pureFeeTable;

    /** Fee table used by XTSE. */
    TsxFeeTable m_tsxFeeTable;

    /** Fee table used by XTSX. */
    TsxFeeTable m_tsxVentureTable;

    /** Fee table used by XATS. */
    XatsFeeTable m_xatsFeeTable;

    /** Fee table used by XCX2. */
    Xcx2FeeTable m_xcx2FeeTable;

    /** The set of ETFs. */
    std::unordered_set<Security> m_etfs;

    /** The set of interlisted Securities. */
    std::unordered_set<Security> m_interlisted;

    /** The set of NEX listed Securities. */
    std::unordered_set<Security> m_nexListed;
  };

  /**
   * Parses the set of TMX interlisted symbols.
   * @param path The path to the YAML file to parse.
   * @param marketDatabase The MarketDatabase used to parse the symbols.
   * @return The set of interlisted symbols.
   */
  inline std::unordered_set<Security> ParseTmxInterlistedSecurities(
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
   * Parses the set of TMX ETF symbols.
   * @param path The path to the YAML file to parse.
   * @param marketDatabase The MarketDatabase used to parse the symbols.
   * @return The set of ETF symbols.
   */
  inline std::unordered_set<Security> ParseTmxEtfSecurities(
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
   * Parses the set of NEX listed symbols.
   * @param path The path to the YAML file to parse.
   * @param marketDatabase The MarketDatabase used to parse the symbols.
   * @return The set of NEX listed symbols.
   */
  inline std::unordered_set<Security> ParseNexListedSecurities(
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
   * Parses a ConsolidatedTmxFeeTable from a YAML configuration.
   * @param config The configuration to parse the ConsolidatedTmxFeeTable from.
   * @param marketDatabase The MarketDatabase used to parse Securities.
   * @return The ConsolidatedTmxFeeTable represented by the <i>config</i>.
   */
  inline ConsolidatedTmxFeeTable ParseConsolidatedTmxFeeTable(
      const YAML::Node& config, const MarketDatabase& marketDatabase) {
    auto feeTable = ConsolidatedTmxFeeTable();
    auto etfPath = Beam::Extract<std::string>(config, "etf_path");
    feeTable.m_etfs = ParseTmxEtfSecurities(etfPath, marketDatabase);
    auto interlistedPath = Beam::Extract<std::string>(config,
      "interlisted_path");
    feeTable.m_interlisted = ParseTmxInterlistedSecurities(interlistedPath,
      marketDatabase);
    auto nexPath = Beam::Extract<std::string>(config, "nex_path");
    feeTable.m_nexListed = ParseNexListedSecurities(nexPath, marketDatabase);
    feeTable.m_spireFee = Beam::Extract<Money>(config, "spire_fee");
    feeTable.m_iirocFee = Beam::Extract<Money>(config, "iiroc_fee");
    feeTable.m_cdsFee = Beam::Extract<Money>(config, "cds_fee");
    feeTable.m_cdsCap = Beam::Extract<int>(config, "cds_cap");
    feeTable.m_clearingFee = Beam::Extract<Money>(config, "clearing_fee");
    feeTable.m_perOrderFee = Beam::Extract<Money>(config, "per_order_fee");
    feeTable.m_perOrderCap = Beam::Extract<Money>(config, "per_order_cap");
    if(auto xatsConfig = config["xats"]) {
      feeTable.m_xatsFeeTable = ParseXatsFeeTable(xatsConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for XATS missing."));
    }
    if(auto chicConfig = config["chic"]) {
      feeTable.m_chicFeeTable = ParseChicFeeTable(chicConfig, feeTable.m_etfs,
        feeTable.m_interlisted);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for CHIC missing."));
    }
    if(auto cseConfig = config["cse"]) {
      feeTable.m_cseFeeTable = ParseCseFeeTable(cseConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for CSE missing."));
    }
    if(auto cse2Config = config["cse2"]) {
      feeTable.m_cse2FeeTable = ParseCse2FeeTable(cse2Config);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for CSE2 missing."));
    }
    if(auto xcx2Config = config["xcx2"]) {
      feeTable.m_xcx2FeeTable = ParseXcx2FeeTable(xcx2Config, feeTable.m_etfs);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for XCX2 missing."));
    }
    if(auto cxdConfig = config["cxd"]) {
      feeTable.m_cxdFeeTable = ParseCxdFeeTable(cxdConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for CXD missing."));
    }
    if(auto lynxConfig = config["lynx"]) {
      feeTable.m_lynxFeeTable = ParseLynxFeeTable(
        lynxConfig, feeTable.m_etfs, feeTable.m_interlisted);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for LYNX missing."));
    }
    if(auto matnConfig = config["matn"]) {
      feeTable.m_matnFeeTable = ParseMatnFeeTable(matnConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for MATN missing."));
    }
    if(auto neoeConfig = config["neoe"]) {
      feeTable.m_neoeFeeTable = ParseNeoeFeeTable(neoeConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for NEOE missing."));
    }
    if(auto nexConfig = config["nex"]) {
      feeTable.m_nexFeeTable = ParseNexFeeTable(nexConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for NEX missing."));
    }
    if(auto omgaConfig = config["omga"]) {
      feeTable.m_omgaFeeTable = ParseOmgaFeeTable(omgaConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for OMGA missing."));
    }
    if(auto pureConfig = config["pure"]) {
      feeTable.m_pureFeeTable = ParsePureFeeTable(pureConfig, marketDatabase);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for PURE missing."));
    }
    if(auto tsxConfig = config["tsx"]) {
      feeTable.m_tsxFeeTable = ParseTsxFeeTable(tsxConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for XTSE missing."));
    }
    if(auto tsxvConfig = config["tsxv"]) {
      feeTable.m_tsxVentureTable = ParseTsxFeeTable(tsxvConfig);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Fee table for XTSX missing."));
    }
    return feeTable;
  }

  /**
   * Calculates the fee on a trade executed on a TMX market.
   * @param feeTable The ConsolidatedTmxFeeTable used to calculate the fee.
   * @param state The historical State of fee calculations.
   * @param order The Order that was traded against.
   * @param executionReport The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline OrderExecutionService::ExecutionReport CalculateFee(
      const ConsolidatedTmxFeeTable& feeTable,
      ConsolidatedTmxFeeTable::State& state,
      const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = executionReport;
    feesReport.m_processingFee += feesReport.m_lastQuantity *
      feeTable.m_clearingFee;
    if(feesReport.m_lastQuantity != 0) {
      auto& fillCount = state.m_fillCount.Get(order.GetInfo().m_orderId);
      ++fillCount;
      feesReport.m_processingFee += feeTable.m_iirocFee;
      if(fillCount <= feeTable.m_cdsCap) {
        feesReport.m_processingFee += feeTable.m_cdsFee;
      }
    }
    feesReport.m_commission += feesReport.m_lastQuantity * feeTable.m_spireFee;
    auto& perOrderCharge = state.m_perOrderCharges.Get(
      order.GetInfo().m_orderId);
    auto perOrderDelta = executionReport.m_lastQuantity *
      feeTable.m_perOrderFee;
    if(perOrderCharge + perOrderDelta > feeTable.m_perOrderCap) {
      perOrderDelta = feeTable.m_perOrderCap - perOrderCharge;
    }
    perOrderCharge += perOrderDelta;
    feesReport.m_processingFee += perOrderDelta;
    feesReport.m_executionFee += [&] {
      auto lastMarket = [&] {
        if(!executionReport.m_lastMarket.empty()) {
          return executionReport.m_lastMarket;
        } else {
          auto& destination = order.GetInfo().m_fields.m_destination;
          if(destination == DefaultDestinations::ALPHA()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::XATS());
          } else if(destination == DefaultDestinations::CHIX()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::CHIC());
          } else if(destination == DefaultDestinations::CSE()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::CSE());
          } else if(destination == DefaultDestinations::CSE2()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::CSE2());
          } else if(destination == DefaultDestinations::CX2()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::XCX2());
          } else if(destination == DefaultDestinations::LYNX()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::LYNX());
          } else if(destination == DefaultDestinations::MATNLP()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::MATN());
          } else if(destination == DefaultDestinations::MATNMF()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::MATN());
          } else if(destination == DefaultDestinations::NEOE()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::NEOE());
          } else if(destination == DefaultDestinations::OMEGA()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::OMGA());
          } else if(destination == DefaultDestinations::PURE()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::PURE());
          } else if(destination == DefaultDestinations::TSX()) {
            return boost::lexical_cast<std::string>(DefaultMarkets::TSX());
          } else {
            return std::string();
          }
        }
      }();
      if(lastMarket == DefaultMarkets::XATS()) {
        auto isEtf = Beam::Contains(feeTable.m_etfs,
          order.GetInfo().m_fields.m_security);
        return CalculateFee(feeTable.m_xatsFeeTable, isEtf, executionReport);
      } else if(lastMarket == DefaultMarkets::CHIC()) {
        return CalculateFee(feeTable.m_chicFeeTable, order.GetInfo().m_fields,
          executionReport);
      } else if(lastMarket == DefaultMarkets::CSE()) {
        return CalculateFee(feeTable.m_cseFeeTable, executionReport);
      } else if(lastMarket == DefaultMarkets::CSE2()) {
        return CalculateFee(
          feeTable.m_cse2FeeTable, order.GetInfo().m_fields, executionReport);
      } else if(lastMarket == DefaultMarkets::XCX2()) {
        return CalculateFee(feeTable.m_xcx2FeeTable, order.GetInfo().m_fields,
          executionReport);
      } else if(lastMarket == DefaultMarkets::CXD()) {
        auto securityClass = [&] {
          if(Beam::Contains(feeTable.m_etfs,
              order.GetInfo().m_fields.m_security)) {
            return CxdFeeTable::SecurityClass::ETF;
          } else {
            return CxdFeeTable::SecurityClass::DEFAULT;
          }
        }();
        return CalculateFee(
          feeTable.m_cxdFeeTable, securityClass, executionReport);
      } else if(lastMarket == DefaultMarkets::LYNX()) {
        return CalculateFee(
          feeTable.m_lynxFeeTable, order.GetInfo().m_fields, executionReport);
      } else if(lastMarket == DefaultMarkets::MATN()) {
        auto classification = [&] {
          if(Beam::Contains(feeTable.m_etfs,
              order.GetInfo().m_fields.m_security)) {
            return MatnFeeTable::Classification::ETF;
          } else {
            return MatnFeeTable::Classification::DEFAULT;
          }
        }();
        return CalculateFee(feeTable.m_matnFeeTable, classification,
          executionReport);
      } else if(lastMarket == DefaultMarkets::NEOE()) {
        auto classification = [&] {
          if(Beam::Contains(
              feeTable.m_interlisted, order.GetInfo().m_fields.m_security)) {
            return NeoeFeeTable::Classification::INTERLISTED;
          } else if(Beam::Contains(
              feeTable.m_etfs, order.GetInfo().m_fields.m_security)) {
            return NeoeFeeTable::Classification::ETF;
          } else {
            return NeoeFeeTable::Classification::GENERAL;
          }
        }();
        return CalculateFee(feeTable.m_neoeFeeTable, classification,
          order.GetInfo().m_fields, executionReport);
      } else if(lastMarket == DefaultMarkets::OMGA()) {
        auto isEtf = Beam::Contains(feeTable.m_etfs,
          order.GetInfo().m_fields.m_security);
        return CalculateFee(feeTable.m_omgaFeeTable, isEtf,
          order.GetInfo().m_fields, executionReport);
      } else if(lastMarket == DefaultMarkets::PURE()) {
        auto section = [&] {
          if(Beam::Contains(
              feeTable.m_etfs, order.GetInfo().m_fields.m_security)) {
            return PureFeeTable::Section::ETF;
          } else if(Beam::Contains(
              feeTable.m_interlisted, order.GetInfo().m_fields.m_security)) {
            return PureFeeTable::Section::INTERLISTED;
          } else {
            return PureFeeTable::Section::DEFAULT;
          }
        }();
        return CalculateFee(feeTable.m_pureFeeTable, section, executionReport);
      } else if(lastMarket == DefaultMarkets::TSX() ||
          lastMarket == DefaultMarkets::TSXV()) {
        if(lastMarket == DefaultMarkets::TSXV() && feeTable.m_nexListed.count(
            order.GetInfo().m_fields.m_security) != 0) {
          return CalculateFee(feeTable.m_nexFeeTable, executionReport);
        }
        auto classification = [&] {
          if(Beam::Contains(feeTable.m_etfs,
              order.GetInfo().m_fields.m_security)) {
            return TsxFeeTable::Classification::ETF;
          } else if(Beam::Contains(feeTable.m_interlisted,
              order.GetInfo().m_fields.m_security)) {
            return TsxFeeTable::Classification::INTERLISTED;
          } else {
            return TsxFeeTable::Classification::DEFAULT;
          }
        }();
        if(order.GetInfo().m_fields.m_security.GetMarket() ==
            DefaultMarkets::TSX()) {
          return CalculateFee(feeTable.m_tsxFeeTable, classification,
            order.GetInfo().m_fields, executionReport);
        } else if(order.GetInfo().m_fields.m_security.GetMarket() ==
            DefaultMarkets::TSXV()) {
          return CalculateFee(feeTable.m_tsxVentureTable, classification,
            order.GetInfo().m_fields, executionReport);
        } else {
          std::cout << "Unknown market [TMX]: \"" <<
            order.GetInfo().m_fields.m_security.GetMarket() << "\"\n";
          return CalculateFee(feeTable.m_tsxFeeTable, classification,
            order.GetInfo().m_fields, executionReport);
        }
      } else {
        std::cout << "Unknown last market [TMX]: \"" << lastMarket << "\"\n";
        return Money::ZERO;
      }
    }();
    return feesReport;
  }
}

#endif
