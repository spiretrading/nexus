#ifndef NEXUS_CHARTING_SERVICE_APPLICATION_DEFINITIONS_HPP
#define NEXUS_CHARTING_SERVICE_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/ChartingService/ServiceChartingClient.hpp"

namespace Nexus {

  /** Encapsulates a standard ChartingClient used in an application. */
  using ApplicationChartingClient = Beam::ApplicationClient<
    ServiceChartingClient, Beam::ServiceName<CHARTING_SERVICE_NAME>,
    Beam::ZLibSessionBuilder<>>;
}

#endif
