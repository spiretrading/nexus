#ifndef NEXUS_ORDER_RECORD_HPP
#define NEXUS_ORDER_RECORD_HPP
#include <ostream>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
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

template<>
class std::formatter<Nexus::OrderRecord> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::OrderRecord& value, auto& context) const {
      auto reports = std::string();
      for(auto& report : value.m_execution_reports) {
        if(!reports.empty()) {
          reports += ", ";
        }
        reports += std::format(context.locale(), "{}", report);
      }
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} [{}])", value.m_info, reports),
        context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, const OrderRecord& value) {
    auto locale = out.getloc();
    if(auto database = static_cast<const CurrencyDatabase*>(
        out.pword(Beam::ScopedStreamManipulator<CurrencyDatabase>::ID))) {
      locale = make_locale(locale, *database);
    }
    if(auto database = static_cast<const VenueDatabase*>(
        out.pword(Beam::ScopedStreamManipulator<VenueDatabase>::ID))) {
      locale = make_locale(locale, *database);
    }
    return out << std::format(locale, "{}", value);
  }
}

#endif
