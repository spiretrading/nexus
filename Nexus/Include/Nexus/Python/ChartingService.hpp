#ifndef NEXUS_PYTHON_CHARTING_SERVICE_HPP
#define NEXUS_PYTHON_CHARTING_SERVICE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported ChartingClient. */
  NEXUS_EXPORT_DLL pybind11::class_<ChartingClient>&
    get_exported_charting_client();

  /**
   * Exports a ChartingClient class.
   * @param <C> The type of ChartingClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ChartingClient.
   */
  template<IsChartingClient C>
  auto export_charting_client(pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C>(module, name.data()).
      def("query", &C::query).
      def("load_price_series", &C::load_price_series).
      def("close", &C::close);
    if constexpr(!std::is_same_v<C, ChartingClient>) {
      pybind11::implicitly_convertible<C, ChartingClient>();
      get_exported_charting_client().
        def(pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
    }
    return client;
  }

  /**
   * Exports the ChartingService namespace.
   * @param module The module to export to.
   */
  void export_charting_service(pybind11::module& module);

  /**
   * Exports the application definitions.
   * @param module The module to export to.
   */
  void export_charting_service_application_definitions(
    pybind11::module& module);

  /**
   * Exports the ChartingServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_charting_service_test_environment(pybind11::module& module);

  /**
   * Exports the TickerChartingQuery class.
   * @param module The module to export to.
   */
  void export_ticker_charting_query(pybind11::module& module);
}

#endif
