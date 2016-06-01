#ifndef NEXUS_CONSOLIDATEDTMXFEETABLE_HPP
#define NEXUS_CONSOLIDATEDTMXFEETABLE_HPP
#include <exception>
#include <sstream>
#include <unordered_set>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandling/MatnFeeTable.hpp"
#include "Nexus/FeeHandling/NeoeFeeTable.hpp"
#include "Nexus/FeeHandling/OmgaFeeTable.hpp"
#include "Nexus/FeeHandling/PureFeeTable.hpp"
#include "Nexus/FeeHandling/TsxFeeTable.hpp"
#include "Nexus/FeeHandling/XatsFeeTable.hpp"
#include "Nexus/FeeHandling/Xcx2FeeTable.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /*! \struct ConsolidatedTmxFeeTable
      \brief Consolidates all TMX related market fees together.
   */
  struct ConsolidatedTmxFeeTable {

    //! Fee table used by CHIC.
    ChicFeeTable m_chicFeeTable;

    //! Fee table used by LYNX.
    LynxFeeTable m_lynxFeeTable;

    //! Fee table used by MATN.
    MatnFeeTable m_matnFeeTable;

    //! Fee table used by NEOE.
    NeoeFeeTable m_neoeFeeTable;

    //! Fee table used by OMGA.
    OmgaFeeTable m_omgaFeeTable;

    //! Fee table used by PURE.
    PureFeeTable m_pureFeeTable;

    //! Fee table used by XTSE.
    TsxFeeTable m_tsxFeeTable;

    //! Fee table used by XTSX.
    TsxFeeTable m_tsxVentureTable;

    //! Fee table used by XATS.
    XatsFeeTable m_xatsFeeTable;

    //! Fee table used by XCX2.
    Xcx2FeeTable m_xcx2FeeTable;

    //! The set of ETFs.
    std::unordered_set<Security> m_etfs;

    //! The set of interlisted Securities.
    std::unordered_set<Security> m_interlisted;
  };

  //! Parses the set of symbols from a YAML config.
  /*!
    \param config The config to parse.
    \param marketDatabase The MarketDatabase used to parse the symbols.
    \return The set of symbols.
  */
  inline std::unordered_set<Security> ParseSecuritySet(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    std::unordered_set<Security> securities;
    for(auto& item : config) {
      auto security = ParseSecurity(item.to<std::string>(), marketDatabase);
      securities.insert(security);
    }
    return securities;
  }

  //! Parses the set of TMX interlisted symbols.
  /*!
    \param path The path to the YAML file to parse.
    \param marketDatabase The MarketDatabase used to parse the symbols.
    \return The set of interlisted symbols.
  */
  inline std::unordered_set<Security> ParseTmxInterlistedSecurities(
      const std::string& path, const MarketDatabase& marketDatabase) {
    std::ifstream configStream{path.c_str()};
    if(!configStream.good()) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "TMX interlisted file not found: \"" + path + "\""});
    }
    YAML::Node config;
    try {
      YAML::Parser configParser(configStream);
      configParser.GetNextDocument(config);
    } catch(YAML::ParserException& e) {
      std::stringstream message;
      message << "Invalid YAML at line " << (e.mark.line + 1) << ", " <<
        "column " << (e.mark.column + 1) << ": " << e.msg << std::endl;
      BOOST_THROW_EXCEPTION(std::runtime_error{message.str()});
    }
    auto symbols = config.FindValue("symbols");
    if(symbols == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "Interlisted symbols not found."});
    }
    return ParseSecuritySet(*symbols, marketDatabase);
  }

  //! Parses the set of TMX ETF symbols.
  /*!
    \param path The path to the YAML file to parse.
    \param marketDatabase The MarketDatabase used to parse the symbols.
    \return The set of ETF symbols.
  */
  inline std::unordered_set<Security> ParseTmxEtfSecurities(
      const std::string& path, const MarketDatabase& marketDatabase) {
    std::ifstream configStream{path.c_str()};
    if(!configStream.good()) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "TMX ETF file not found: \"" + path + "\""});
    }
    YAML::Node config;
    try {
      YAML::Parser configParser(configStream);
      configParser.GetNextDocument(config);
    } catch(YAML::ParserException& e) {
      std::stringstream message;
      message << "Invalid YAML at line " << (e.mark.line + 1) << ", " <<
        "column " << (e.mark.column + 1) << ": " << e.msg << std::endl;
      BOOST_THROW_EXCEPTION(std::runtime_error{message.str()});
    }
    auto symbols = config.FindValue("symbols");
    if(symbols == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "Interlisted symbols not found."});
    }
    return ParseSecuritySet(*symbols, marketDatabase);
  }

  //! Parses a ConsolidatedTmxFeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the ConsolidatedTmxFeeTable from.
    \param marketDatabase The MarketDatabase used to parse Securities.
    \return The ConsolidatedTmxFeeTable represented by the <i>config</i>.
  */
  inline ConsolidatedTmxFeeTable ParseConsolidatedTmxFeeTable(
      const YAML::Node& config, const MarketDatabase& marketDatabase) {
    ConsolidatedTmxFeeTable feeTable;
    auto xatsConfig = config.FindValue("xats");
    if(xatsConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for XATS missing."});
    } else {
      feeTable.m_xatsFeeTable = ParseXatsFeeTable(*xatsConfig);
    }
    auto chicConfig = config.FindValue("chic");
    if(chicConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for CHIC missing."});
    } else {
      feeTable.m_chicFeeTable = ParseChicFeeTable(*chicConfig);
    }
    auto xcx2Config = config.FindValue("xcx2");
    if(xcx2Config == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for XCX2 missing."});
    } else {
      feeTable.m_xcx2FeeTable = ParseXcx2FeeTable(*xcx2Config);
    }
    auto lynxConfig = config.FindValue("lynx");
    if(lynxConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for LYNX missing."});
    } else {
      feeTable.m_lynxFeeTable = ParseLynxFeeTable(*lynxConfig, marketDatabase);
    }
    auto matnConfig = config.FindValue("matn");
    if(matnConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for MATN missing."});
    } else {
      feeTable.m_matnFeeTable = ParseMatnFeeTable(*matnConfig);
    }
    auto neoeConfig = config.FindValue("neoe");
    if(neoeConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for NEOE missing."});
    } else {
      feeTable.m_neoeFeeTable = ParseNeoeFeeTable(*neoeConfig);
    }
    auto omgaConfig = config.FindValue("omga");
    if(omgaConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for OMGA missing."});
    } else {
      feeTable.m_omgaFeeTable = ParseOmgaFeeTable(*omgaConfig);
    }
    auto pureConfig = config.FindValue("pure");
    if(pureConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for PURE missing."});
    } else {
      feeTable.m_pureFeeTable = ParsePureFeeTable(*pureConfig, marketDatabase);
    }
    auto tsxConfig = config.FindValue("tsx");
    if(tsxConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for XTSE missing."});
    } else {
      feeTable.m_tsxFeeTable = ParseTsxFeeTable(*tsxConfig);
    }
    auto tsxvConfig = config.FindValue("tsxv");
    if(tsxvConfig == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Fee table for XTSX missing."});
    } else {
      feeTable.m_tsxVentureTable = ParseTsxFeeTable(*tsxvConfig);
    }
    auto etfPath = Beam::Extract<std::string>(config, "etf_path");
    feeTable.m_etfs = ParseTmxEtfSecurities(etfPath, marketDatabase);
    auto interlistedPath = Beam::Extract<std::string>(config,
      "interlisted_path");
    feeTable.m_interlisted = ParseTmxInterlistedSecurities(interlistedPath,
      marketDatabase);
    return feeTable;
  }

  //! Calculates the fee on a trade executed on a TMX market.
  /*!
    \param feeTable The ConsolidatedTmxFeeTable used to calculate the fee.
    \param order The Order that was traded against.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return The fee calculated for the specified trade.
  */
  inline Money CalculateFee(const ConsolidatedTmxFeeTable& feeTable,
      const OrderExecutionService::Order& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto lastMarket = [&] {
      if(!executionReport.m_lastMarket.empty()) {
        return std::string{executionReport.m_lastMarket};
      } else {
        auto& destination = order.GetInfo().m_fields.m_destination;
        if(destination == DefaultDestinations::ALPHA()) {
          return ToString(DefaultMarkets::XATS());
        } else if(destination == DefaultDestinations::CHIX()) {
          return ToString(DefaultMarkets::CHIC());
        } else if(destination == DefaultDestinations::CX2()) {
          return ToString(DefaultMarkets::XCX2());
        } else if(destination == DefaultDestinations::LYNX()) {
          return ToString(DefaultMarkets::LYNX());
        } else if(destination == DefaultDestinations::MATNLP()) {
          return ToString(DefaultMarkets::MATN());
        } else if(destination == DefaultDestinations::MATNMF()) {
          return ToString(DefaultMarkets::MATN());
        } else if(destination == DefaultDestinations::NEOE()) {
          return ToString(DefaultMarkets::NEOE());
        } else if(destination == DefaultDestinations::OMEGA()) {
          return ToString(DefaultMarkets::OMGA());
        } else if(destination == DefaultDestinations::PURE()) {
          return ToString(DefaultMarkets::PURE());
        } else if(destination == DefaultDestinations::TSX()) {
          return ToString(DefaultMarkets::TSX());
        } else {
          return std::string{};
        }
      }
    }();
    if(lastMarket == DefaultMarkets::XATS()) {
      auto isEtf = Beam::Contains(feeTable.m_etfs,
        order.GetInfo().m_fields.m_security);
      return CalculateFee(feeTable.m_xatsFeeTable, isEtf, executionReport);
    } else if(lastMarket == DefaultMarkets::CHIC()) {
      auto isEtf = Beam::Contains(feeTable.m_etfs,
        order.GetInfo().m_fields.m_security);
      auto isInterlisted = Beam::Contains(feeTable.m_interlisted,
        order.GetInfo().m_fields.m_security);
      return CalculateFee(feeTable.m_chicFeeTable, isEtf, isInterlisted,
        order.GetInfo().m_fields, executionReport);
    } else if(lastMarket == DefaultMarkets::XCX2()) {
      return CalculateFee(feeTable.m_xcx2FeeTable, order.GetInfo().m_fields,
        executionReport);
    } else if(lastMarket == DefaultMarkets::LYNX()) {
      return CalculateFee(feeTable.m_lynxFeeTable,
        order.GetInfo().m_fields.m_security, executionReport);
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
      auto isInterlisted = Beam::Contains(feeTable.m_interlisted,
        order.GetInfo().m_fields.m_security);
      return CalculateFee(feeTable.m_neoeFeeTable, isInterlisted,
        order.GetInfo().m_fields, executionReport);
    } else if(lastMarket == DefaultMarkets::OMGA()) {
      auto isEtf = Beam::Contains(feeTable.m_etfs,
        order.GetInfo().m_fields.m_security);
      return CalculateFee(feeTable.m_omgaFeeTable, isEtf,
        order.GetInfo().m_fields, executionReport);
    } else if(lastMarket == DefaultMarkets::PURE()) {
      return CalculateFee(feeTable.m_pureFeeTable,
        order.GetInfo().m_fields.m_security, executionReport);
    } else if(lastMarket == DefaultMarkets::TSX() ||
        lastMarket == DefaultMarkets::TSXV()) {
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
          order.GetInfo().m_fields.m_security.GetMarket() << "\"";
        return CalculateFee(feeTable.m_tsxFeeTable, classification,
          order.GetInfo().m_fields, executionReport);
      }
    } else {
      std::cout << "Unknown last market [TMX]: \"" << lastMarket << "\"";
      return Money::ZERO;
    }
  }
}

#endif
