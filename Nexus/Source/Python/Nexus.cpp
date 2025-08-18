#include "Nexus/Python/Nexus.hpp"
#include <pybind11/pybind11.h>

using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

PYBIND11_MODULE(nexus, m) {
  auto beam_module = module::import("beam");
//  ExportQueries(m);
  export_accounting(m);
//  ExportAdministrationService(m);
//  ExportChartingService(m);
//  ExportCompliance(m);
  export_definitions(m);
//  ExportDefinitionsService(m);
//  ExportFeeHandling(m);
//  ExportMarketDataService(m);
//  ExportOrderExecutionService(m);
//  ExportRiskService(m);
//  ExportServiceClients(m);
//  ExportBacktester(m);
//  ExportTechnicalAnalysis(m);
//  ExportTelemetryService(m);
}
