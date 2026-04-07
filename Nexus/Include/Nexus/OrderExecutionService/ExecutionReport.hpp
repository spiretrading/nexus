#ifndef NEXUS_EXECUTION_REPORT_HPP
#define NEXUS_EXECUTION_REPORT_HPP
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/Tag.hpp"

namespace Nexus {

  /** Type of integer used for order ids. */
  using OrderId = std::uint64_t;

  /** Represents an update to an Order. */
  struct ExecutionReport {

    /** The id of the Order being reported. */
    OrderId m_id;

    /** The timestamp. */
    boost::posix_time::ptime m_timestamp;

    /** The sequence. */
    int m_sequence;

    /** The status of the Order. */
    OrderStatus m_status;

    /** The quantity last executed. */
    Quantity m_last_quantity;

    /** The price of the last execution. */
    Money m_last_price;

    /** The liquidity flag. */
    std::string m_liquidity_flag;

    /** The last market. */
    std::string m_last_market;

    /** The execution fee. */
    Money m_execution_fee;

    /** The processing fee. */
    Money m_processing_fee;

    /** The commission. */
    Money m_commission;

    /** The text. */
    std::string m_text;

    /** Any additional tags. */
    std::vector<Tag> m_additional_tags;

    /** Constructs an ExecutionReport. */
    ExecutionReport() noexcept;

    /**
     * Constructs an ExecutionReport for an Order in a PENDING_NEW status.
     * @param id The id of the Order this ExecutionReport belongs to.
     * @param timestamp The timestamp.
     */
    ExecutionReport(OrderId id, boost::posix_time::ptime timestamp) noexcept;

    bool operator ==(const ExecutionReport&) const = default;
  };

  /**
   * Returns an ExecutionReport updating the OrderStatus of a previous
   * ExecutionReport.
   * @param report The ExecutionReport to update.
   * @param status The new OrderStatus.
   * @param timestamp The timestamp.
   */
  inline ExecutionReport make_update(const ExecutionReport& report,
      OrderStatus status, boost::posix_time::ptime timestamp) {
    auto updated_report = ExecutionReport();
    updated_report.m_id = report.m_id;
    updated_report.m_timestamp = timestamp;
    updated_report.m_sequence = report.m_sequence + 1;
    updated_report.m_status = status;
    return updated_report;
  }

  /**
   * Returns the sum of all fees.
   * @param report The ExecutionReport to calculate.
   * @return The sum of all of the <i>report</i>'s fees.
   */
  inline Money get_fee_total(const ExecutionReport& report) {
    return report.m_execution_fee + report.m_processing_fee +
      report.m_commission;
  }

  inline std::ostream& operator <<(
      std::ostream& out, const ExecutionReport& report) {
    out << '(' << report.m_id << ' ' << report.m_timestamp << ' ' <<
      report.m_sequence << ' ' << report.m_status << ' ' <<
      report.m_last_quantity << ' ' << report.m_last_price << ' ' <<
      report.m_liquidity_flag << ' ' << report.m_last_market << ' ' <<
      report.m_execution_fee << ' ' << report.m_processing_fee << ' ' <<
      report.m_commission << ' ' << report.m_text;
    out << " [";
    auto is_first = true;
    for(auto& tag : report.m_additional_tags) {
      if(!is_first) {
        out << ' ';
      } else {
        is_first = false;
      }
      out << tag;
    }
    return out << "])";
  }

  inline ExecutionReport::ExecutionReport() noexcept
    : m_id(0),
      m_sequence(0),
      m_last_quantity(0) {}

  inline ExecutionReport::ExecutionReport(
    OrderId id, boost::posix_time::ptime timestamp) noexcept
    : m_id(id),
      m_timestamp(timestamp),
      m_sequence(0),
      m_status(OrderStatus::PENDING_NEW),
      m_last_quantity(0) {}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ExecutionReport> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::ExecutionReport& value, unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("timestamp", value.m_timestamp);
      shuttle.shuttle("sequence", value.m_sequence);
      shuttle.shuttle("status", value.m_status);
      shuttle.shuttle("last_quantity", value.m_last_quantity);
      shuttle.shuttle("last_price", value.m_last_price);
      shuttle.shuttle("liquidity_flag", value.m_liquidity_flag);
      shuttle.shuttle("last_market", value.m_last_market);
      shuttle.shuttle("execution_fee", value.m_execution_fee);
      shuttle.shuttle("processing_fee", value.m_processing_fee);
      shuttle.shuttle("commission", value.m_commission);
      shuttle.shuttle("text", value.m_text);
      shuttle.shuttle("additional_tags", value.m_additional_tags);
    }
  };
}

#endif
