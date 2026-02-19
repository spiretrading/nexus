#ifndef NEXUS_CONSOLIDATED_TMX_FEE_TABLE_HPP
#define NEXUS_CONSOLIDATED_TMX_FEE_TABLE_HPP
#include <exception>
#include <sstream>
#include <unordered_set>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandling/Cse2FeeTable.hpp"
#include "Nexus/FeeHandling/CxdFeeTable.hpp"
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
      Beam::SynchronizedUnorderedMap<OrderId, Money> m_per_order_charges;

      /** The number of times an Order has been filled. */
      Beam::SynchronizedUnorderedMap<OrderId, int> m_fill_count;
    };

    /** The fee charged by the software. */
    Money m_spire_fee;

    /** The fee charged by IIROC. */
    Money m_iiroc_fee;

    /** The CDS fee. */
    Money m_cds_fee;

    /** The CDS cap. */
    int m_cds_cap;

    /** The clearing fee; */
    Money m_clearing_fee;

    /** The fee charged per Order traded. */
    Money m_per_order_fee;

    /** The cap charged per Order. */
    Money m_per_order_cap;

    /** Fee table used by CHIC. */
    ChicFeeTable m_chic_fee_table;

    /** Fee table used by CSE. */
    CseFeeTable m_cse_fee_table;

    /** Fee table used by CSE2. */
    Cse2FeeTable m_cse2_fee_table;

    /** Fee table used by CXD. */
    CxdFeeTable m_cxd_fee_table;

    /** Fee table used by LYNX. */
    LynxFeeTable m_lynx_fee_table;

    /** Fee table used by MATN. */
    MatnFeeTable m_matn_fee_table;

    /** Fee table used by NEOE. */
    NeoeFeeTable m_neoe_fee_table;

    /** Fee table used by NEX. */
    NexFeeTable m_nex_fee_table;

    /** Fee table used by OMGA. */
    OmgaFeeTable m_omga_fee_table;

    /** Fee table used by PURE. */
    PureFeeTable m_pure_fee_table;

    /** Fee table used by XTSE. */
    TsxFeeTable m_tsx_fee_table;

    /** Fee table used by XTSX. */
    TsxFeeTable m_tsxv_fee_table;

    /** Fee table used by XATS. */
    XatsFeeTable m_xats_fee_table;

    /** Fee table used by XCX2. */
    Xcx2FeeTable m_xcx2_fee_table;

    /** The set of ETFs. */
    std::unordered_set<Ticker> m_etfs;

    /** The set of interlisted Tickers. */
    std::unordered_set<Ticker> m_interlisted;

    /** The set of NEX listed Tickers. */
    std::unordered_set<Ticker> m_nex_listed;
  };

  /**
   * Parses the set of TMX interlisted symbols.
   * @param path The path to the YAML file to parse.
   * @param venues The VenueDatabase used to parse the symbols.
   * @return The set of interlisted symbols.
   */
  inline std::unordered_set<Ticker> parse_tmx_interlisted_tickers(
      const std::string& path, const VenueDatabase& venues) {
    auto config = Beam::load_file(path);
    auto symbols = config["symbols"];
    if(!symbols) {
      boost::throw_with_location(
        std::runtime_error("Interlisted symbols not found."));
    }
    return parse_ticker_set(symbols, venues);
  }

  /**
   * Parses the set of TMX ETF symbols.
   * @param path The path to the YAML file to parse.
   * @param venues The VenueDatabase used to parse the symbols.
   * @return The set of ETF symbols.
   */
  inline std::unordered_set<Ticker> parse_tmx_etf_tickers(
      const std::string& path, const VenueDatabase& venues) {
    auto config = Beam::load_file(path);
    auto symbols = config["symbols"];
    if(!symbols) {
      boost::throw_with_location(std::runtime_error("ETF symbols not found."));
    }
    return parse_ticker_set(symbols, venues);
  }

  /**
   * Parses the set of NEX listed symbols.
   * @param path The path to the YAML file to parse.
   * @param venues The VenueDatabase used to parse the symbols.
   * @return The set of NEX listed symbols.
   */
  inline std::unordered_set<Ticker> parse_nex_listed_tickers(
      const std::string& path, const VenueDatabase& venues) {
    auto config = Beam::load_file(path);
    auto symbols = config["symbols"];
    if(!symbols) {
      boost::throw_with_location(std::runtime_error("NEX symbols not found."));
    }
    return parse_ticker_set(symbols, venues);
  }

  /**
   * Parses a ConsolidatedTmxFeeTable from a YAML configuration.
   * @param config The configuration to parse the ConsolidatedTmxFeeTable from.
   * @param venues The VenueDatabase used to parse Tickers.
   * @return The ConsolidatedTmxFeeTable represented by the <i>config</i>.
   */
  inline ConsolidatedTmxFeeTable parse_consolidated_tmx_fee_table(
      const YAML::Node& config, const VenueDatabase& venues) {
    auto table = ConsolidatedTmxFeeTable();
    auto etf_path = Beam::extract<std::string>(config, "etf_path");
    table.m_etfs = parse_tmx_etf_tickers(etf_path, venues);
    auto interlisted_path =
      Beam::extract<std::string>(config, "interlisted_path");
    table.m_interlisted =
      parse_tmx_interlisted_tickers(interlisted_path, venues);
    auto nex_path = Beam::extract<std::string>(config, "nex_path");
    table.m_nex_listed = parse_nex_listed_tickers(nex_path, venues);
    table.m_spire_fee = Beam::extract<Money>(config, "spire_fee");
    table.m_iiroc_fee = Beam::extract<Money>(config, "iiroc_fee");
    table.m_cds_fee = Beam::extract<Money>(config, "cds_fee");
    table.m_cds_cap = Beam::extract<int>(config, "cds_cap");
    table.m_clearing_fee = Beam::extract<Money>(config, "clearing_fee");
    table.m_per_order_fee = Beam::extract<Money>(config, "per_order_fee");
    table.m_per_order_cap = Beam::extract<Money>(config, "per_order_cap");
    if(auto xats_config = config["xats"]) {
      table.m_xats_fee_table = parse_xats_fee_table(xats_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for XATS missing."));
    }
    if(auto chic_config = config["chic"]) {
      table.m_chic_fee_table =
        parse_chic_fee_table(chic_config, table.m_etfs, table.m_interlisted);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for CHIC missing."));
    }
    if(auto cse_config = config["cse"]) {
      table.m_cse_fee_table = parse_cse_fee_table(cse_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for CSE missing."));
    }
    if(auto cse2_config = config["cse2"]) {
      table.m_cse2_fee_table = parse_cse2_fee_table(cse2_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for CSE2 missing."));
    }
    if(auto xcx2_config = config["xcx2"]) {
      table.m_xcx2_fee_table = parse_xcx2_fee_table(xcx2_config, table.m_etfs);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for XCX2 missing."));
    }
    if(auto cxd_config = config["cxd"]) {
      table.m_cxd_fee_table = parse_cxd_fee_table(cxd_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for CXD missing."));
    }
    if(auto lynx_config = config["lynx"]) {
      table.m_lynx_fee_table =
        parse_lynx_fee_table(lynx_config, table.m_etfs, table.m_interlisted);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for LYNX missing."));
    }
    if(auto matn_config = config["matn"]) {
      table.m_matn_fee_table = parse_matn_fee_table(matn_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for MATN missing."));
    }
    if(auto neoe_config = config["neoe"]) {
      table.m_neoe_fee_table = parse_neoe_fee_table(neoe_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for NEOE missing."));
    }
    if(auto nex_config = config["nex"]) {
      table.m_nex_fee_table = parse_nex_fee_table(nex_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for NEX missing."));
    }
    if(auto omga_config = config["omga"]) {
      table.m_omga_fee_table = parse_omga_fee_table(omga_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for OMGA missing."));
    }
    if(auto pure_config = config["pure"]) {
      table.m_pure_fee_table = parse_pure_fee_table(pure_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for PURE missing."));
    }
    if(auto tsx_config = config["tsx"]) {
      table.m_tsx_fee_table = parse_tsx_fee_table(tsx_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for XTSE missing."));
    }
    if(auto tsxv_config = config["tsxv"]) {
      table.m_tsxv_fee_table = parse_tsx_fee_table(tsxv_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for XTSX missing."));
    }
    return table;
  }

  /**
   * Calculates the fee on a trade executed on a TMX market.
   * @param table The ConsolidatedTmxFeeTable used to calculate the fee.
   * @param state The historical State of fee calculations.
   * @param order The Order that was traded against.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline ExecutionReport calculate_fee(const ConsolidatedTmxFeeTable& table,
      ConsolidatedTmxFeeTable::State& state, const Order& order,
      const ExecutionReport& report) {
    auto fees_report = report;
    fees_report.m_processing_fee +=
      fees_report.m_last_quantity * table.m_clearing_fee;
    if(fees_report.m_last_quantity != 0) {
      auto& fill_count = state.m_fill_count.get(order.get_info().m_id);
      ++fill_count;
      fees_report.m_processing_fee += table.m_iiroc_fee;
      if(fill_count <= table.m_cds_cap) {
        fees_report.m_processing_fee += table.m_cds_fee;
      }
    }
    fees_report.m_commission += fees_report.m_last_quantity * table.m_spire_fee;
    auto& per_order_charge =
      state.m_per_order_charges.get(order.get_info().m_id);
    auto per_order_delta = report.m_last_quantity * table.m_per_order_fee;
    if(per_order_charge + per_order_delta > table.m_per_order_cap) {
      per_order_delta = table.m_per_order_cap - per_order_charge;
    }
    per_order_charge += per_order_delta;
    fees_report.m_processing_fee += per_order_delta;
    fees_report.m_execution_fee += [&] {
      auto last_market = [&] {
        if(!report.m_last_market.empty()) {
          return Venue(report.m_last_market);
        } else {
          auto& destination = order.get_info().m_fields.m_destination;
          if(destination == DefaultDestinations::ALPHA) {
            return DefaultVenues::XATS;
          } else if(destination == DefaultDestinations::CHIX) {
            return DefaultVenues::CHIC;
          } else if(destination == DefaultDestinations::CSE) {
            return DefaultVenues::CSE;
          } else if(destination == DefaultDestinations::CSE2) {
            return DefaultVenues::CSE2;
          } else if(destination == DefaultDestinations::CX2) {
            return DefaultVenues::XCX2;
          } else if(destination == DefaultDestinations::LYNX) {
            return DefaultVenues::LYNX;
          } else if(destination == DefaultDestinations::MATNLP) {
            return DefaultVenues::MATN;
          } else if(destination == DefaultDestinations::MATNMF) {
            return DefaultVenues::MATN;
          } else if(destination == DefaultDestinations::NEOE) {
            return DefaultVenues::NEOE;
          } else if(destination == DefaultDestinations::OMEGA) {
            return DefaultVenues::OMGA;
          } else if(destination == DefaultDestinations::PURE) {
            return DefaultVenues::PURE;
          } else if(destination == DefaultDestinations::TSX) {
            return DefaultVenues::TSX;
          } else {
            return Venue();
          }
        }
      }();
      if(last_market == DefaultVenues::XATS) {
        auto is_etf =
          table.m_etfs.contains(order.get_info().m_fields.m_ticker);
        return calculate_fee(table.m_xats_fee_table, is_etf, report);
      } else if(last_market == DefaultVenues::CHIC) {
        return calculate_fee(
          table.m_chic_fee_table, order.get_info().m_fields, report);
      } else if(last_market == DefaultVenues::CSE) {
        auto& ticker = order.get_info().m_fields.m_ticker;
        auto listing = [&] {
          if(table.m_etfs.contains(ticker)) {
            return CseFeeTable::CseListing::ETF;
          } else if(table.m_interlisted.contains(ticker)) {
            return CseFeeTable::CseListing::INTERLISTED;
          } else if(ticker.get_venue() == DefaultVenues::CSE) {
            return CseFeeTable::CseListing::CSE_LISTED;
          }
          return CseFeeTable::CseListing::DEFAULT;
        }();
        return calculate_fee(table.m_cse_fee_table, listing, report);
      } else if(last_market == DefaultVenues::CSE2) {
        return calculate_fee(
          table.m_cse2_fee_table, order.get_info().m_fields, report);
      } else if(last_market == DefaultVenues::XCX2) {
        return calculate_fee(
          table.m_xcx2_fee_table, order.get_info().m_fields, report);
      } else if(last_market == DefaultVenues::CXD) {
        auto ticker_class = [&] {
          if(table.m_etfs.contains(order.get_info().m_fields.m_ticker)) {
            return CxdFeeTable::SecurityClass::ETF;
          } else {
            return CxdFeeTable::SecurityClass::DEFAULT;
          }
        }();
        return calculate_fee(table.m_cxd_fee_table, ticker_class, report);
      } else if(last_market == DefaultVenues::LYNX) {
        return calculate_fee(
          table.m_lynx_fee_table, order.get_info().m_fields, report);
      } else if(last_market == DefaultVenues::MATN) {
        auto classification = [&] {
          if(table.m_etfs.contains(order.get_info().m_fields.m_ticker)) {
            return MatnFeeTable::Classification::ETF;
          } else {
            return MatnFeeTable::Classification::DEFAULT;
          }
        }();
        return calculate_fee(table.m_matn_fee_table, classification, report);
      } else if(last_market == DefaultVenues::NEOE) {
        auto classification = [&] {
          if(table.m_interlisted.contains(
              order.get_info().m_fields.m_ticker)) {
            return NeoeFeeTable::Classification::INTERLISTED;
          } else if(table.m_etfs.contains(
              order.get_info().m_fields.m_ticker)) {
            return NeoeFeeTable::Classification::ETF;
          } else {
            return NeoeFeeTable::Classification::GENERAL;
          }
        }();
        return calculate_fee(table.m_neoe_fee_table, classification,
          order.get_info().m_fields, report);
      } else if(last_market == DefaultVenues::OMGA) {
        auto is_etf =
          table.m_etfs.contains(order.get_info().m_fields.m_ticker);
        return calculate_fee(
          table.m_omga_fee_table, is_etf, order.get_info().m_fields, report);
      } else if(last_market == DefaultVenues::PURE) {
        auto section = [&] {
          if(table.m_etfs.contains(order.get_info().m_fields.m_ticker)) {
            return PureFeeTable::Section::ETF;
          } else if(table.m_interlisted.contains(
              order.get_info().m_fields.m_ticker)) {
            return PureFeeTable::Section::INTERLISTED;
          } else {
            return PureFeeTable::Section::DEFAULT;
          }
        }();
        return calculate_fee(table.m_pure_fee_table, section, report);
      } else if(last_market == DefaultVenues::TSX ||
          last_market == DefaultVenues::TSXV) {
        if(last_market == DefaultVenues::TSXV &&
            table.m_nex_listed.contains(order.get_info().m_fields.m_ticker)) {
          return calculate_fee(table.m_nex_fee_table, report);
        }
        auto classification = [&] {
          if(table.m_etfs.contains(order.get_info().m_fields.m_ticker)) {
            return TsxFeeTable::Classification::ETF;
          } else if(table.m_interlisted.contains(
              order.get_info().m_fields.m_ticker)) {
            return TsxFeeTable::Classification::INTERLISTED;
          } else {
            return TsxFeeTable::Classification::DEFAULT;
          }
        }();
        if(order.get_info().m_fields.m_ticker.get_venue() ==
            DefaultVenues::TSX) {
          return calculate_fee(table.m_tsx_fee_table, classification,
            order.get_info().m_fields, report);
        } else if(order.get_info().m_fields.m_ticker.get_venue() ==
            DefaultVenues::TSXV) {
          return calculate_fee(table.m_tsxv_fee_table, classification,
            order.get_info().m_fields, report);
        } else {
          std::cout << "Unknown market [TMX]: \"" <<
            order.get_info().m_fields.m_ticker.get_venue() << "\"\n";
          return calculate_fee(table.m_tsx_fee_table, classification,
            order.get_info().m_fields, report);
        }
      } else {
        std::cout << "Unknown last market [TMX]: \"" << last_market << "\"\n";
        return Money::ZERO;
      }
    }();
    return fees_report;
  }
}

#endif
