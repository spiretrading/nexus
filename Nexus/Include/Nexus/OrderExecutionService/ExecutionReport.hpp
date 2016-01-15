#ifndef NEXUS_EXECUTIONREPORT_HPP
#define NEXUS_EXECUTIONREPORT_HPP
#include <string>
#include <tuple>
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \struct ExecutionReport
      \brief Represents an update to an Order.
   */
  struct ExecutionReport {

    //! The id of the Order being reported.
    OrderId m_id;

    //! The timestamp.
    boost::posix_time::ptime m_timestamp;

    //! The sequence.
    int m_sequence;

    //! The status of the Order.
    OrderStatus m_status;

    //! The quantity last executed.
    Quantity m_lastQuantity;

    //! The price of the last execution.
    Money m_lastPrice;

    //! The liquidity flag.
    std::string m_liquidityFlag;

    //! The last market.
    std::string m_lastMarket;

    //! The execution fee.
    Money m_executionFee;

    //! The processing fee.
    Money m_processingFee;

    //! The commission.
    Money m_commission;

    //! The text.
    std::string m_text;

    //! Any additional tags.
    std::vector<Tag> m_additionalTags;

    //! Constructs an ExecutionReport.
    ExecutionReport();

    //! Builds an initial ExecutionReport for an Order in a PENDING_NEW status.
    /*!
      \param id The id of the Order this ExecutionReport belongs to.
      \param timestamp The timestamp.
    */
    static ExecutionReport BuildInitialReport(OrderId id,
      const boost::posix_time::ptime& timestamp);

    //! Builds a new ExecutionReport updating the OrderStatus of a previous
    //! ExecutionReport.
    /*!
      \param report The ExecutionReport to update.
      \param status The new OrderStatus.
      \param timestamp The timestamp.
    */
    static ExecutionReport BuildUpdatedReport(const ExecutionReport& report,
      OrderStatus status, const boost::posix_time::ptime& timestamp);
  };

  //! Tests if two ExecutionReports are equal.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <code>lhs</code> is equal to <code>rhs</code>.
  */
  inline bool operator ==(const ExecutionReport& lhs,
      const ExecutionReport& rhs) {
    return std::tie(lhs.m_id, lhs.m_timestamp, lhs.m_sequence, lhs.m_status,
      lhs.m_lastQuantity, lhs.m_lastPrice, lhs.m_liquidityFlag,
      lhs.m_lastMarket, lhs.m_executionFee, lhs.m_processingFee,
      lhs.m_commission, lhs.m_text, lhs.m_additionalTags) ==
      std::tie(rhs.m_id, rhs.m_timestamp, rhs.m_sequence, rhs.m_status,
      rhs.m_lastQuantity, rhs.m_lastPrice, rhs.m_liquidityFlag,
      rhs.m_lastMarket, rhs.m_executionFee, rhs.m_processingFee,
      rhs.m_commission, rhs.m_text, rhs.m_additionalTags);
  }

  //! Tests if two ExecutionReports are not equal.
  /*!
    \param lhs The left hand side of the inequality.
    \param rhs The right hand side of the inequality.
    \return <code>true</code> iff <i>lhs</i> is not equal to <i>rhs</i>.
  */
  inline bool operator !=(const ExecutionReport& lhs,
      const ExecutionReport& rhs) {
    return !(lhs == rhs);
  }

  inline ExecutionReport::ExecutionReport()
      : m_id(0),
        m_lastQuantity(0) {}

  inline ExecutionReport ExecutionReport::BuildInitialReport(OrderId id,
      const boost::posix_time::ptime& timestamp) {
    ExecutionReport report;
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

  inline ExecutionReport ExecutionReport::BuildUpdatedReport(
      const ExecutionReport& report, OrderStatus status,
      const boost::posix_time::ptime& timestamp) {
    ExecutionReport updatedReport = report;
    updatedReport.m_timestamp = timestamp;
    updatedReport.m_sequence = report.m_sequence + 1;
    updatedReport.m_status = status;
    updatedReport.m_lastQuantity = 0;
    updatedReport.m_lastPrice = Money::ZERO;
    updatedReport.m_executionFee = Money::ZERO;
    updatedReport.m_processingFee = Money::ZERO;
    updatedReport.m_commission = Money::ZERO;
    updatedReport.m_additionalTags.clear();
    return updatedReport;
  }
}
}

namespace Beam {
namespace Serialization {
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
}

#endif
