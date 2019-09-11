#include "Nexus/Python/Nexus.hpp"
#include <pybind11/pybind11.h>

using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

PYBIND11_MODULE(nexus, module) {
  ExportAccounting(module);
  ExportAdministrationService(module);
  ExportCompliance(module);
  ExportDefinitions(module);
  ExportDefinitionsService(module);
  ExportFeeHandling(module);
  ExportMarketDataService(module);
  ExportQueries(module);
  ExportRiskService(module);
  ExportServiceClients(module);
  ExportBacktester(module);
}
