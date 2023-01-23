#ifndef NEXUS_PYTHON_TECHNICAL_ANALYSIS_HPP
#define NEXUS_PYTHON_TECHNICAL_ANALYSIS_HPP
#include <type_traits>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include "Nexus/Python/DllExport.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"

namespace Nexus::Python {

  /** Returns the exported Candlestick. */
  NEXUS_EXPORT_DLL pybind11::class_<
    TechnicalAnalysis::Candlestick<pybind11::object, pybind11::object>>&
      GetExportedCandlestick();

  /**
   * Exports the generic Candlestick class.
   * @param module The module to export to.
   */
  template<typename Candlestick>
  auto ExportCandlestick(pybind11::module& module, const std::string& name) {
    auto candlestick = pybind11::class_<Candlestick>(module, name.c_str()).
      def(pybind11::init()).
      def(pybind11::init<
        typename Candlestick::Domain, typename Candlestick::Domain>()).
      def(pybind11::init<
        typename Candlestick::Domain, typename Candlestick::Domain,
        typename Candlestick::Range, typename Candlestick::Range,
        typename Candlestick::Range, typename Candlestick::Range>()).
      def_property("start", &Candlestick::GetStart, &Candlestick::SetStart).
      def_property("end", &Candlestick::GetEnd, &Candlestick::SetEnd).
      def_property_readonly("open", &Candlestick::GetOpen).
      def_property_readonly("close", &Candlestick::GetClose).
      def_property_readonly("high", &Candlestick::GetHigh).
      def_property_readonly("low", &Candlestick::GetLow).
      def("update", &Candlestick::Update);
    if constexpr(!std::is_same_v<Candlestick,
        TechnicalAnalysis::Candlestick<pybind11::object, pybind11::object>>) {
      candlestick.def(pybind11::self == pybind11::self).
        def(pybind11::self != pybind11::self);
      pybind11::implicitly_convertible<Candlestick,
        TechnicalAnalysis::Candlestick<pybind11::object, pybind11::object>>();
      GetExportedCandlestick().def(pybind11::init(
        [] (const Candlestick& candlestick) {
          return TechnicalAnalysis::Candlestick(
            pybind11::cast(candlestick.GetStart()),
            pybind11::cast(candlestick.GetEnd()),
            pybind11::cast(candlestick.GetOpen()),
            pybind11::cast(candlestick.GetClose()),
            pybind11::cast(candlestick.GetHigh()),
            pybind11::cast(candlestick.GetLow()));
        }));
    }
    return candlestick;
  }

  /**
   * Exports the TechnicalAnalysis namespace.
   * @param module The module to export to.
   */
  void ExportTechnicalAnalysis(pybind11::module& module);
}

#endif
