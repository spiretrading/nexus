#ifndef SPIRE_EXECUTION_REPORTS_MODEL_HPP
#define SPIRE_EXECUTION_REPORTS_MODEL_HPP
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** An observable list of execution report entries. */
  using ExecutionReportsModel = ListModel<Nexus::ExecutionReportEntry>;
}

#endif
