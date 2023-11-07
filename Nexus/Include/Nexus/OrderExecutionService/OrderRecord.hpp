#ifndef NEXUS_ORDER_RECORD_HPP
#define NEXUS_ORDER_RECORD_HPP
#include <ostream>
#include <tuple>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Streamable.hpp>
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

    bool operator ==(const OrderRecord& rhs) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const OrderRecord& value) {
    return out << '(' << value.m_info << ' ' <<
      Beam::Stream(value.m_executionReports) << ')';
  }
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
