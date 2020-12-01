#ifndef NEXUS_PYTHON_CHARTING_SERVICE_HPP
#define NEXUS_PYTHON_CHARTING_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/ChartingService/ChartingClientBox.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported ChartingClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<ChartingService::ChartingClientBox>&
    GetExportedChartingClientBox();

  /**
   * Exports the ApplicationChartingClient class.
   * @param module The module to export to.
   */
  void ExportApplicationChartingClient(pybind11::module& module);

  /**
   * Exports the ChartingService namespace.
   * @param module The module to export to.
   */
  void ExportChartingService(pybind11::module& module);

  /**
   * Exports the ChartingServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportChartingServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports the SecurityChartingQuery class.
   * @param module The module to export to.
   */
  void ExportSecurityChartingQuery(pybind11::module& module);

  /**
   * Exports a ChartingClient class.
   * @param <Client> The type of ChartingClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ChartingClient.
   */
  template<typename Client>
  auto ExportChartingClient(pybind11::module& module,
      const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("query_security", &Client::QuerySecurity).
      def("load_time_price_series", &Client::LoadTimePriceSeries).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client, ChartingService::ChartingClientBox>) {
      pybind11::implicitly_convertible<Client,
        ChartingService::ChartingClientBox>();
      GetExportedChartingClientBox().def(pybind11::init<
        std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
