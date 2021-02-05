#ifndef NEXUS_MATCH_REPORT_BUILDER_HPP
#define NEXUS_MATCH_REPORT_BUILDER_HPP
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Concept.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::InternalMatcher {

  /**
   * Interface for building ExecutionReports that are the result of an internal
   * match.
   */
  struct MatchReportBuilder : Beam::Concept<MatchReportBuilder> {

    /**
     * Returns ExecutionReports that are the result of an internal match.
     * @param passiveOrderFields The OrderFields represented by the passive side
     *        of the internal match.
     * @param activeOrderFields The OrderFields represented by the active side
     *        of the internal match.
     * @param passiveExecutionReport Stores the ExecutionReport of the passive
     *        order.
     * @param activeExecutionReport Stores the ExecutionReport of the active
     *        order.
     */
    void Make(const OrderExecutionService::OrderFields& passiveOrderFields,
      const OrderExecutionService::OrderFields& activeOrderFields,
      Beam::Out<OrderExecutionService::ExecutionReport> passiveExecutionReport,
      Beam::Out<OrderExecutionService::ExecutionReport> activeExecutionReport);
  };
}

#endif
