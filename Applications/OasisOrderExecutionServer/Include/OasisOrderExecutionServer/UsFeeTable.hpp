#ifndef OASIS_US_FEE_TABLE_HPP
#define OASIS_US_FEE_TABLE_HPP
#include <iostream>
#include "Nexus/FeeHandling/OtcmFeeTable.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /** Stores the fees among U.S. trading venues. */
  struct UsFeeTable {

    /** The fee charged by the software. */
    Money m_spire_fee;

    /** Fee table used by OTCM. */
    OtcmFeeTable m_otcm_fee_table;
  };

  /**
   * Parses a UsFeeTable from a YAML configuration.
   * @param config The configuration to parse the UsFeeTable from.
   * @param venues The VenueDatabase used to parse Securities.
   * @return The UsFeeTable represented by the <i>config</i>.
   */
  inline UsFeeTable parse_us_fee_table(const YAML::Node& config) {
    auto table = UsFeeTable();
    table.m_spire_fee = Beam::extract<Money>(config, "spire_fee");
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
    fees_report.m_commission += fees_report.m_last_quantity * table.m_spire_fee;
    fees_report.m_execution_fee += [&] {
      auto last_market = [&] {
        if(!report.m_last_market.empty()) {
          return Venue(report.m_last_market);
        } else {
          auto& destination = order.get_info().m_fields.m_destination;
          if(destination == DefaultDestinations::OTCM) {
            return DefaultVenues::OTCM;
          } else {
            return Venue();
          }
        }
      }();
      if(last_market == DefaultVenues::OTCM) {
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
