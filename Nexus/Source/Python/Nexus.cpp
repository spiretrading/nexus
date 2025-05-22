#include "Nexus/Python/Nexus.hpp"
#include <pybind11/pybind11.h>

using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

PYBIND11_MODULE(nexus, m) {
  auto beamModule = module::import("beam");
  ExportQueries(m);
  ExportAccounting(m);
  ExportAdministrationService(m);
  ExportChartingService(m);
  ExportCompliance(m);
  ExportDefinitions(m);
  ExportDefinitionsService(m);
  ExportFeeHandling(m);
  ExportMarketDataService(m);
  ExportOrderExecutionService(m);
  ExportRiskService(m);
  ExportServiceClients(m);
  ExportBacktester(m);
  ExportTechnicalAnalysis(m);
  ExportTelemetryService(m);
}
