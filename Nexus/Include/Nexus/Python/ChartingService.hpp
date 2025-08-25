#ifndef NEXUS_PYTHON_CHARTING_SERVICE_HPP
#define NEXUS_PYTHON_CHARTING_SERVICE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/ChartingService/ChartingClient.hpp"

namespace Nexus::Python {

  /**
   * Exports a ChartingClient class.
   * @param <C> The type of ChartingClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ChartingClient.
   */
  template<IsChartingClient C>
  auto export_charting_client(pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C, std::shared_ptr<C>>(module, name.data()).
      def("query", &C::query).
      def("load_time_price_series", &C::load_time_price_series).
      def("close", &C::close);
    return client;
  }

  /**
   * Exports the ChartingService namespace.
   * @param module The module to export to.
   */
  void export_charting_service(pybind11::module& module);

  /**
   * Exports the ChartingServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_charting_service_test_environment(pybind11::module& module);

  /**
   * Exports the SecurityChartingQuery class.
   * @param module The module to export to.
   */
  void export_security_charting_query(pybind11::module& module);
}

#endif
