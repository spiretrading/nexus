#include "Nexus/Python/PythonBindings.hpp"
#include <Beam/Python/BoostPython.hpp>
#include "Nexus/Python/Accounting.hpp"
#include "Nexus/Python/AdministrationService.hpp"
#include "Nexus/Python/Backtester.hpp"
#include "Nexus/Python/Compliance.hpp"
#include "Nexus/Python/Definitions.hpp"
#include "Nexus/Python/DefinitionsService.hpp"
#include "Nexus/Python/FeeHandling.hpp"
#include "Nexus/Python/MarketDataService.hpp"
#include "Nexus/Python/OrderExecutionService.hpp"
#include "Nexus/Python/Queries.hpp"
#include "Nexus/Python/RiskService.hpp"
#include "Nexus/Python/ServiceClients.hpp"
#include "Nexus/Python/Tasks.hpp"

using namespace Nexus;
using namespace Nexus::Python;
using namespace boost;
using namespace boost::python;

namespace {
  void Finalize() {
  }
}

BOOST_PYTHON_MODULE(nexus) {
  PyEval_InitThreads();
  def("_finalize", &Finalize);
  auto atexit = object{handle<>(PyImport_ImportModule("atexit"))};
  object finalize = scope().attr("_finalize");
  atexit.attr("register")(finalize);
  ExportAccounting();
  ExportAdministrationService();
  ExportCompliance();
  ExportDefinitions();
  ExportDefinitionsService();
  ExportFeeHandling();
  ExportMarketDataService();
  ExportOrderExecutionService();
  ExportQueries();
  ExportRiskService();
  ExportServiceClients();
  ExportBacktester();
  ExportTasks();
}
