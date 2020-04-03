#ifndef NEXUS_ORDER_RECORD_HPP
#define NEXUS_ORDER_RECORD_HPP
#include <ostream>
#include <tuple>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"

namespace Nexus::OrderExecutionService {

  /** brief Stores information about a single Order. */
  struct OrderRecord {

    /** The Order's submission info. */
    OrderInfo m_info;

    /** The list of ExecutionReports. */
    std::vector<ExecutionReport> m_executionReports;

    /** Constructs an empty OrderRecord. */
    OrderRecord() = default;

    /**
     * Constructs an OrderRecord.
     * @param info The Order's submission info.
     * @param executionReports The list of ExecutionReports.
     */
    OrderRecord(OrderInfo info, std::vector<ExecutionReport> executionReports);
  };

  //! Tests if two OrderRecords are equal.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <code>lhs</code> is equal to <code>rhs</code>.
  */
  inline bool operator ==(const OrderRecord& lhs, const OrderRecord& rhs) {
    return std::tie(lhs.m_info, lhs.m_executionReports) ==
      std::tie(rhs.m_info, rhs.m_executionReports);
  }

  //! Tests if two OrderRecords are not equal.
  /*!
    \param lhs The left hand side of the inequality.
    \param rhs The right hand side of the inequality.
    \return <code>true</code> iff <i>lhs</i> is not equal to <i>rhs</i>.
  */
  inline bool operator !=(const OrderRecord& lhs, const OrderRecord& rhs) {
    return !(lhs == rhs);
  }

  inline std::ostream& operator <<(std::ostream& out,
      const OrderRecord& value) {
    return ::operator <<(out << '(' << value.m_info << ' ',
      value.m_executionReports) << ')';
  }

  inline OrderRecord::OrderRecord(OrderInfo info,
    std::vector<ExecutionReport> executionReports)
    : m_info(std::move(info)),
      m_executionReports(std::move(executionReports)) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::OrderExecutionService::OrderRecord> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::OrderExecutionService::OrderRecord& value,
        unsigned int version) {
      shuttle.Shuttle("info", value.m_info);
      shuttle.Shuttle("execution_reports", value.m_executionReports);
    }
  };
}

#endif
