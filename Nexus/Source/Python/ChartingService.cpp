#include "Nexus/Python/ChartingService.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/ChartingService/ApplicationDefinitions.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonChartingClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::ChartingService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  auto chartingClientBox = std::unique_ptr<class_<ChartingClientBox>>();
}

class_<ChartingClientBox>& Nexus::Python::GetExportedChartingClientBox() {
  return *chartingClientBox;
}

void Nexus::Python::ExportApplicationChartingClient(module& module) {
  using PythonApplicationChartingClient = ToPythonChartingClient<
    ChartingClient<ZLibSessionBuilder<ServiceLocatorClientBox>>>;
  ExportChartingClient<PythonApplicationChartingClient>(module,
    "ApplicationChartingClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<PythonApplicationChartingClient>(
        MakeSessionBuilder<ZLibSessionBuilder<ServiceLocatorClientBox>>(
          std::move(serviceLocatorClient), ChartingService::SERVICE_NAME));
    }));
}

void Nexus::Python::ExportChartingService(module& module) {
  auto submodule = module.def_submodule("charting_service");
  chartingClientBox = std::make_unique<class_<ChartingClientBox>>(
    ExportChartingClient<ChartingClientBox>(submodule, "ChartingClient"));
  ExportChartingClient<ToPythonChartingClient<ChartingClientBox>>(submodule,
    "ChartingClientBox");
  ExportApplicationChartingClient(submodule);
  ExportSecurityChartingQuery(submodule);
  ExportQueueSuite<QueryVariant>(submodule, "QueryVariant");
  auto test_module = submodule.def_submodule("tests");
  ExportChartingServiceTestEnvironment(test_module);
}

void Nexus::Python::ExportChartingServiceTestEnvironment(module& module) {
  class_<ChartingServiceTestEnvironment>(module,
    "ChartingServiceTestEnvironment").
    def(init<ServiceLocatorClientBox, MarketDataClientBox>(),
      call_guard<GilRelease>()).
    def("__del__",
      [] (ChartingServiceTestEnvironment& self) {
        self.Close();
      }, call_guard<GilRelease>()).
    def("close", &ChartingServiceTestEnvironment::Close,
      call_guard<GilRelease>()).
    def("make_client",
      [] (ChartingServiceTestEnvironment& self,
          ServiceLocatorClientBox serviceLocatorClient) {
        return ToPythonChartingClient(self.MakeClient(
          std::move(serviceLocatorClient)));
      }, call_guard<GilRelease>());
}

void Nexus::Python::ExportSecurityChartingQuery(module& module) {
  class_<SecurityChartingQuery, BasicQuery<Security>, ExpressionQuery>(module,
    "SecurityChartingQuery").
    def_property("market_data_type",
      &SecurityChartingQuery::GetMarketDataType,
      &SecurityChartingQuery::SetMarketDataType);
}
