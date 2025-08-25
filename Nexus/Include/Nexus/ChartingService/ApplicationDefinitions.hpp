#ifndef NEXUS_CHARTING_SERVICE_APPLICATION_DEFINITIONS_HPP
#define NEXUS_CHARTING_SERVICE_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/ChartingService/ServiceChartingClient.hpp"

namespace Nexus {

  /** Encapsulates a standard ChartingClient used in an application. */
  using ApplicationChartingClient = Beam::Services::ApplicationClient<
    ServiceChartingClient, Beam::Services::ServiceName<CHARTING_SERVICE_NAME>,
    Beam::Services::ZLibSessionBuilder<>>;
}

#endif
