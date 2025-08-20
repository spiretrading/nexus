#include "Nexus/Python/ChartingService.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/Python/ToPythonChartingClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::ChartingService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_charting_service(module& module) {
  auto submodule = module.def_submodule("charting_service");
  export_charting_client<ToPythonChartingClient<ChartingClient>>(
    submodule, "ChartingClient");
  export_security_charting_query(submodule);
  auto tests_submodule = submodule.def_submodule("tests");
  export_charting_service_test_environment(tests_submodule);
}

void Nexus::Python::export_charting_service_test_environment(module& module) {
  class_<ChartingServiceTestEnvironment>(
    module, "ChartingServiceTestEnvironment").
      def(init<ServiceLocatorClientBox, MarketDataClient>(),
        call_guard<GilRelease>(), arg("service_locator_client"),
        arg("market_data_client")).
      def("__del__", [] (ChartingServiceTestEnvironment& self) {
        self.close();
      }, call_guard<GilRelease>()).
      def("make_client", [] (ChartingServiceTestEnvironment& self,
          ServiceLocatorClientBox service_locator_client) {
        return ToPythonChartingClient(
          self.make_client(std::move(service_locator_client)));
      }, call_guard<gil_scoped_release>(), arg("service_locator_client")).
      def("close", &ChartingServiceTestEnvironment::close,
        call_guard<gil_scoped_release>());
}

void Nexus::Python::export_security_charting_query(module& module) {
  class_<SecurityChartingQuery, BasicQuery<Security>, ExpressionQuery>(
    module, "SecurityChartingQuery").
      def(init()).
      def(init<const SecurityChartingQuery&>()).
      def_property("market_data_type",
        &SecurityChartingQuery::get_market_data_type,
        &SecurityChartingQuery::set_market_data_type);
}
