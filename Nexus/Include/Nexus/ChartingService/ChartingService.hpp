#ifndef NEXUS_CHARTING_SERVICE_HPP
#define NEXUS_CHARTING_SERVICE_HPP
#include <string>

namespace Nexus::ChartingService {
  class ApplicationChartingClient;
  template<typename B> class ChartingClient;
  template<typename C, typename M> class ChartingServlet;
  class SecurityChartingQuery;
  class VirtualChartingClient;
  template<typename C> class WrapperChartingClient;

  /** Standard name for the charting service. */
  inline const std::string SERVICE_NAME = "charting_service";
}

#endif
