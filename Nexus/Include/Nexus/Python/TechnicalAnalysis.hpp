#ifndef NEXUS_PYTHON_TECHNICAL_ANALYSIS_HPP
#define NEXUS_PYTHON_TECHNICAL_ANALYSIS_HPP
#include <string_view>
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include "Beam/Python/Utilities.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"

namespace Nexus::Python {

  /**
   * Exports a Candlestick class.
   * @param C The type of Candlestick to export.
   * @param name The name of the class to export.
   * @param module The module to export to.
   * @return The exported class.
   */
  template<typename C>
  auto export_candlestick(pybind11::module& module, std::string_view name) {
    using DomainType = typename C::Domain;
    using RangeType = typename C::Range;
    auto candlestick = Beam::Python::export_default_methods(
        pybind11::class_<C>(module, name.data())).
      def(pybind11::init<DomainType, DomainType>()).
      def(pybind11::init<DomainType, DomainType, RangeType, RangeType,
        RangeType, RangeType, Quantity>()).
      def_property("start", &C::get_start, &C::set_start).
      def_property("end", &C::get_end, &C::set_end).
      def_property_readonly("open", &C::get_open).
      def_property_readonly("close", &C::get_close).
      def_property_readonly("high", &C::get_high).
      def_property_readonly("low", &C::get_low).
      def_property_readonly("volume", &C::get_volume).
      def("update", pybind11::overload_cast<RangeType>(&C::update)).
      def("update", pybind11::overload_cast<RangeType, Quantity>(&C::update));
    return candlestick;
  }

  /**
   * Exports the TechnicalAnalysis namespace.
   * @param module The module to export to.
   */
  void export_technical_analysis(pybind11::module& module);

  /**
   * Exports the standard ticker queries.
   * @param module The module to export to.
   */
  void export_standard_ticker_queries(pybind11::module& module);
}

#endif
