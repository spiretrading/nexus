#ifndef NEXUS_ORDER_EXECUTION_APPLICATION_DEFINITIONS_HPP
#define NEXUS_ORDER_EXECUTION_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/OrderExecutionService/ServiceOrderExecutionClient.hpp"

namespace Nexus {

  /** Encapsulates a standard OrderExecutionClient used in an application. */
  using ApplicationOrderExecutionClient = Beam::ApplicationClient<
    ServiceOrderExecutionClient,
    Beam::ServiceName<ORDER_EXECUTION_SERVICE_NAME>>;
}

#endif
