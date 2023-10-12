#ifndef NEXUS_EXECUTION_REPORT_HPP
#define NEXUS_EXECUTION_REPORT_HPP
#include <ostream>
#include <string>
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

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
    Quantity m_lastQuantity;

    /** The price of the last execution. */
    Money m_lastPrice;

    /** The liquidity flag. */
    std::string m_liquidityFlag;

    /** The last market. */
    std::string m_lastMarket;

    /** The execution fee. */
    Money m_executionFee;

    /** The processing fee. */
    Money m_processingFee;

    /** The commission. */
    Money m_commission;

    /** The text. */
    std::string m_text;

    /** Any additional tags. */
    std::vector<Tag> m_additionalTags;

    /** Constructs an ExecutionReport. */
    ExecutionReport();

    /**
     * Returns an initial ExecutionReport for an Order in a PENDING_NEW status.
     * @param id The id of the Order this ExecutionReport belongs to.
     * @param timestamp The timestamp.
     */
    static ExecutionReport MakeInitialReport(
      OrderId id, boost::posix_time::ptime timestamp);

    /**
     * Returns an ExecutionReport updating the OrderStatus of a previous
     * ExecutionReport.
     * @param report The ExecutionReport to update.
     * @param status The new OrderStatus.
     * @param timestamp The timestamp.
     */
    static ExecutionReport MakeUpdatedReport(const ExecutionReport& report,
      OrderStatus status, boost::posix_time::ptime timestamp);

    bool operator ==(const ExecutionReport& rhs) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const ExecutionReport& report) {
    out << '(' << report.m_id << ' ' << report.m_timestamp << ' ' <<
      report.m_sequence << ' ' << report.m_status << ' ' <<
      report.m_lastQuantity << ' ' << report.m_lastPrice << ' ' <<
      report.m_liquidityFlag << ' ' << report.m_lastMarket << ' ' <<
      report.m_executionFee << ' ' << report.m_processingFee << ' ' <<
      report.m_commission << ' ' << report.m_text;
    out << " [";
    auto isFirst = true;
    for(auto& tag : report.m_additionalTags) {
      if(!isFirst) {
        out << ' ';
      } else {
        isFirst = false;
      }
      out << tag;
    }
    return out << "])";
  }

  /**
   * Returns the sum of all fees.
   * @param report The ExecutionReport to calculate.
   * @return The sum of all of the <i>report</i>'s fees.
   */
  inline Money GetFeeTotal(const ExecutionReport& report) {
    return report.m_executionFee + report.m_processingFee + report.m_commission;
  }

  inline ExecutionReport::ExecutionReport()
    : m_id(0),
      m_lastQuantity(0) {}

  inline ExecutionReport ExecutionReport::MakeInitialReport(
      OrderId id, boost::posix_time::ptime timestamp) {
    auto report = ExecutionReport();
    report.m_id = id;
    report.m_timestamp = timestamp;
    report.m_sequence = 0;
    report.m_status = OrderStatus::PENDING_NEW;
    report.m_lastQuantity = 0;
    report.m_lastPrice = Money::ZERO;
    report.m_executionFee = Money::ZERO;
    report.m_processingFee = Money::ZERO;
    report.m_commission = Money::ZERO;
    return report;
  }

  inline ExecutionReport ExecutionReport::MakeUpdatedReport(
      const ExecutionReport& report, OrderStatus status,
      boost::posix_time::ptime timestamp) {
    auto updatedReport = ExecutionReport();
    updatedReport.m_id = report.m_id;
    updatedReport.m_timestamp = timestamp;
    updatedReport.m_sequence = report.m_sequence + 1;
    updatedReport.m_status = status;
    return updatedReport;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::OrderExecutionService::ExecutionReport> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::OrderExecutionService::ExecutionReport& value,
        unsigned int version) {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("timestamp", value.m_timestamp);
      shuttle.Shuttle("sequence", value.m_sequence);
      shuttle.Shuttle("status", value.m_status);
      shuttle.Shuttle("last_quantity", value.m_lastQuantity);
      shuttle.Shuttle("last_price", value.m_lastPrice);
      shuttle.Shuttle("liquidity_flag", value.m_liquidityFlag);
      shuttle.Shuttle("last_market", value.m_lastMarket);
      shuttle.Shuttle("execution_fee", value.m_executionFee);
      shuttle.Shuttle("processing_fee", value.m_processingFee);
      shuttle.Shuttle("commission", value.m_commission);
      shuttle.Shuttle("text", value.m_text);
      shuttle.Shuttle("additional_tags", value.m_additionalTags);
    }
  };
}

#endif
