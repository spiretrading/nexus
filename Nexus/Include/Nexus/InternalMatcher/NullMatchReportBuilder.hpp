#ifndef NEXUS_NULL_MATCH_REPORT_BUILDER_HPP
#define NEXUS_NULL_MATCH_REPORT_BUILDER_HPP
#include "Nexus/InternalMatcher/InternalMatcher.hpp"
#include "Nexus/InternalMatcher/MatchReportBuilder.hpp"

namespace Nexus::InternalMatcher {

  /**
   * Implements the MatchReportBuilder by leaving the ExecutionReports as is.
   */
  class NullMatchReportBuilder {
    public:
      void Make(const OrderExecutionService::OrderFields& passiveOrderFields,
        const OrderExecutionService::OrderFields& activeOrderFields,
        Beam::Out<OrderExecutionService::ExecutionReport>
        passiveExecutionReport,
        Beam::Out<OrderExecutionService::ExecutionReport>
        activeExecutionReport);
  };

  inline void NullMatchReportBuilder::Make(
      const OrderExecutionService::OrderFields& passiveOrderFields,
      const OrderExecutionService::OrderFields& activeOrderFields,
      Beam::Out<OrderExecutionService::ExecutionReport> passiveExecutionReport,
      Beam::Out<OrderExecutionService::ExecutionReport>
      activeExecutionReport) {
    activeExecutionReport->m_text = "Internal match.";
    passiveExecutionReport->m_text = "Internal match.";
  }
}

#endif
