#ifndef NEXUS_NULLMATCHREPORTBUILDER_HPP
#define NEXUS_NULLMATCHREPORTBUILDER_HPP
#include "Nexus/InternalMatcher/InternalMatcher.hpp"
#include "Nexus/InternalMatcher/MatchReportBuilder.hpp"

namespace Nexus {
namespace InternalMatcher {

  /*! \class NullMatchReportBuilder.
      \brief Implements the MatchReportBuilder by leaving the ExecutionReports
             as is.
   */
  class NullMatchReportBuilder {
    public:

      //! Constructs a NullMatchReportBuilder.
      NullMatchReportBuilder();

      void Build(const OrderExecutionService::OrderFields& passiveOrderFields,
        const OrderExecutionService::OrderFields& activeOrderFields,
        Beam::Out<OrderExecutionService::ExecutionReport>
        passiveExecutionReport,
        Beam::Out<OrderExecutionService::ExecutionReport>
        activeExecutionReport);
  };

  inline NullMatchReportBuilder::NullMatchReportBuilder() {}

  inline void NullMatchReportBuilder::Build(
      const OrderExecutionService::OrderFields& passiveOrderFields,
      const OrderExecutionService::OrderFields& activeOrderFields,
      Beam::Out<OrderExecutionService::ExecutionReport> passiveExecutionReport,
      Beam::Out<OrderExecutionService::ExecutionReport>
      activeExecutionReport) {
    activeExecutionReport->m_text = "Internal match.";
    passiveExecutionReport->m_text = "Internal match.";
  }
}
}

#endif
