#include "Nexus/Python/ChartingService.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/ChartingService/ApplicationDefinitions.hpp"
#include "Nexus/ChartingService/TickerChartingQuery.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/Python/ToPythonChartingClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace Nexus::Python;
using namespace Nexus::Tests;
using namespace pybind11;

namespace {
  auto charting_client = std::unique_ptr<class_<ChartingClient>>();
}

class_<ChartingClient>& Nexus::Python::get_exported_charting_client() {
  return *charting_client;
}

void Nexus::Python::export_charting_service(module& module) {
  charting_client = std::make_unique<class_<ChartingClient>>(
    export_charting_client<ChartingClient>(module, "ChartingClient"));
  export_charting_service_application_definitions(module);
  export_ticker_charting_query(module);
  auto tests_submodule = module.def_submodule("tests");
  export_charting_service_test_environment(tests_submodule);
}

void Nexus::Python::export_charting_service_application_definitions(
    module& module) {
  export_charting_client<ToPythonChartingClient<ApplicationChartingClient>>(
    module, "ApplicationChartingClient").
    def(pybind11::init(
      [] (ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>&
          client) {
        return std::make_unique<ToPythonChartingClient<
          ApplicationChartingClient>>(Ref(client.get()));
      }), keep_alive<1, 2>());
}

void Nexus::Python::export_charting_service_test_environment(module& module) {
  class_<ChartingServiceTestEnvironment,
      std::shared_ptr<ChartingServiceTestEnvironment>>(module,
        "ChartingServiceTestEnvironment").
    def(pybind11::init(&make_python_shared<ChartingServiceTestEnvironment,
      ServiceLocatorClient&, MarketDataClient&>), keep_alive<1, 2>(),
      keep_alive<1, 3>()).
    def("make_client",
      [] (ChartingServiceTestEnvironment& self, ServiceLocatorClient& client) {
        return ToPythonChartingClient(self.make_client(Ref(client)));
      }, call_guard<GilRelease>(), keep_alive<0, 2>()).
    def("close", &ChartingServiceTestEnvironment::close,
      call_guard<GilRelease>());
}

void Nexus::Python::export_ticker_charting_query(module& module) {
  export_default_methods(
    class_<TickerChartingQuery, BasicQuery<Ticker>, ExpressionQuery>(
      module, "TickerChartingQuery")).
    def_property("market_data_type", &TickerChartingQuery::get_market_data_type,
      &TickerChartingQuery::set_market_data_type);
}
