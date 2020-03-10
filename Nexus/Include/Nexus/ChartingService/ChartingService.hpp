#ifndef NEXUS_CHARTING_SERVICE_HPP
#define NEXUS_CHARTING_SERVICE_HPP
#include <string>

namespace Nexus::ChartingService {
  class ApplicationChartingClient;
  template<typename ServiceProtocolClientBuilderType> class ChartingClient;
  template<typename ContainerType, typename MarketDataClientType>
    class ChartingServlet;
  class SecurityChartingQuery;
  class VirtualChartingClient;
  template<typename ClientType> class WrapperChartingClient;

  // Standard name for the charting service.
  inline const std::string SERVICE_NAME = "charting_service";
}

#endif
