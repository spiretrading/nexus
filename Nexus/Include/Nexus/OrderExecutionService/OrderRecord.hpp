#ifndef NEXUS_ORDER_RECORD_HPP
#define NEXUS_ORDER_RECORD_HPP
#include <ostream>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"

namespace Nexus {

  /** brief Stores information about a single Order. */
  struct OrderRecord {

    /** The Order's submission info. */
    OrderInfo m_info;

    /** The list of ExecutionReports. */
    std::vector<ExecutionReport> m_execution_reports;

    bool operator ==(const OrderRecord&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const OrderRecord& value) {
    return out << '(' << value.m_info << ' ' <<
      Beam::Stream(value.m_execution_reports) << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::OrderRecord> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::OrderRecord& value, unsigned int version) const {
      shuttle.shuttle("info", value.m_info);
      shuttle.shuttle("execution_reports", value.m_execution_reports);
    }
  };
}

#endif
