#ifndef NEXUS_MATCHREPORTBUILDER_HPP
#define NEXUS_MATCHREPORTBUILDER_HPP
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Concept.hpp>

namespace Nexus {
namespace InternalMatcher {

  /*! \struct MatchReportBuilder
      \brief Interface for building ExecutionReports that are the result of an
             internal match.
   */
  struct MatchReportBuilder : Beam::Concept<MatchReportBuilder> {

    //! Builds ExecutionReports that are the result of an internal match.
    /*!
      \param passiveOrderFields The OrderFields represented by the passive side
             of the internal match.
      \param activeOrderFields The OrderFields represented by the active side of
             the internal match.
      \param passiveExecutionReport Stores the ExecutionReport of the passive
             order.
      \param activeExecutionReport Stores the ExecutionReport of the active
             order.
    */
    void Build(const OrderExecutionService::OrderFields& passiveOrderFields,
      const OrderExecutionService::OrderFields& activeOrderFields,
      Beam::Out<OrderExecutionService::ExecutionReport> passiveExecutionReport,
      Beam::Out<OrderExecutionService::ExecutionReport> activeExecutionReport);
  };
}
}

#endif
