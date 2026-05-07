#ifndef OASIS_US_FEE_TABLE_HPP
#define OASIS_US_FEE_TABLE_HPP
#include <iostream>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/FeeHandling/OtcmFeeTable.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the fees among U.S. trading venues. */
  struct UsFeeTable {

    /** The clearing fee per share. */
    Money m_clearing_fee;

    /** The SEC fee rate applied to the notional value of sell executions. */
    boost::rational<int> m_sec_rate;

    /** The Spire fee rate applied to the notional value of all executions. */
    boost::rational<int> m_spire_rate;

    /** Fee table used by OTCM. */
    OtcmFeeTable m_otcm_fee_table;
  };

  /**
   * Parses a UsFeeTable from a YAML configuration.
   * @param config The configuration to parse the UsFeeTable from.
   * @return The UsFeeTable represented by the <i>config</i>.
   */
  inline UsFeeTable parse_us_fee_table(const YAML::Node& config) {
    auto table = UsFeeTable();
    table.m_clearing_fee = Beam::extract<Money>(config, "clearing_fee");
    table.m_sec_rate = Beam::extract<boost::rational<int>>(config, "sec_rate");
    table.m_spire_rate =
      Beam::extract<boost::rational<int>>(config, "spire_rate");
    if(auto otcm_config = config["otcm"]) {
      table.m_otcm_fee_table = parse_otcm_fee_table(otcm_config);
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table for OTCM missing."));
    }
    return table;
  }

  /**
   * Calculates the fee on a trade executed on a US venue.
   * @param table The UsFeeTable used to calculate the fee.
   * @param order The Order that was traded against.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline ExecutionReport calculate_fee(const UsFeeTable& table,
      const Order& order, const ExecutionReport& report) {
    auto fees_report = report;
    auto notional = report.m_last_quantity * report.m_last_price;
    fees_report.m_processing_fee +=
      report.m_last_quantity * table.m_clearing_fee;
    if(order.get_info().m_fields.m_side == Side::ASK) {
      fees_report.m_processing_fee +=
        round_to(table.m_sec_rate * notional, Money::CENT);
    }
    fees_report.m_commission +=
      round_to(table.m_spire_rate * notional, Money::CENT);
    fees_report.m_execution_fee += [&] {
      auto last_market = [&] {
        if(!report.m_last_market.empty()) {
          return Venue(report.m_last_market);
        } else {
          auto& destination = order.get_info().m_fields.m_destination;
          if(destination == Destinations::OTCM) {
            return Venues::OTCM;
          } else {
            return Venue();
          }
        }
      }();
      if(last_market == Venues::OTCM) {
        return calculate_fee(
          table.m_otcm_fee_table, order.get_info().m_fields, report);
      } else {
        std::cout << "Unknown last market [US]: \"" << last_market << "\"\n";
        return Money::ZERO;
      }
    }();
    return fees_report;
  }
}

#endif
