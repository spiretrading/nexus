#ifndef NEXUS_CHARTINGSERVICE_HPP
#define NEXUS_CHARTINGSERVICE_HPP
#include <string>

namespace Nexus {
namespace ChartingService {
  class ApplicationChartingClient;
  template<typename ServiceProtocolClientBuilderType> class ChartingClient;
  template<typename ContainerType, typename MarketDataClientType>
    class ChartingServlet;
  class SecurityChartingQuery;
  class VirtualChartingClient;
  template<typename ClientType> class WrapperChartingClient;

  // Standard name for the charting service.
  static const std::string SERVICE_NAME = "charting_service";
}
}

#endif
