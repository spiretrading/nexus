#ifndef NEXUS_CHARTING_SERVICE_HPP
#define NEXUS_CHARTING_SERVICE_HPP
#include <string>

namespace Nexus::ChartingService {
  template<typename B> class ChartingClient;
  class ChartingClientBox;
  template<typename C, typename M> class ChartingServlet;
  class SecurityChartingQuery;

  /** Standard name for the charting service. */
  inline const auto SERVICE_NAME = std::string("charting_service");
}

#endif
