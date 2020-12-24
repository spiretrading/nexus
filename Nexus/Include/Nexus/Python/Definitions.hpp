#ifndef NEXUS_PYTHON_DEFINITIONS_HPP
#define NEXUS_PYTHON_DEFINITIONS_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the BboQuote struct.
   * @param module The module to export to.
   */
  void ExportBboQuote(pybind11::module& module);

  /**
   * Exports the BookQuote struct.
   * @param module The module to export to.
   */
  void ExportBookQuote(pybind11::module& module);

  /**
   * Exports the CountryDatabase class.
   * @param module The module to export to.
   */
  void ExportCountry(pybind11::module& module);

  /**
   * Exports the CurrencyDatabase class.
   * @param module The module to export to.
   */
  void ExportCurrency(pybind11::module& module);

  /**
   * Exports the CurrencyPair class.
   * @param module The module to export to.
   */
  void ExportCurrencyPair(pybind11::module& module);

  /**
   * Exports the DefaultCountries namespace.
   * @param module The module to export to.
   */
  void ExportDefaultCountries(pybind11::module& module);

  /**
   * Exports the DefaultCurrencies namespace.
   * @param module The module to export to.
   */
  void ExportDefaultCurrencies(pybind11::module& module);

  /**
   * Exports the DefaultDestinations namespace.
   * @param module The module to export to.
   */
  void ExportDefaultDestinations(pybind11::module& module);

  /**
   * Exports the DefaultMarkets namespace.
   * @param module The module to export to.
   */
  void ExportDefaultMarkets(pybind11::module& module);

  /**
   * Exports the Definitions classes.
   * @param module The module to export to.
   */
  void ExportDefinitions(pybind11::module& module);

  /**
   * Exports the DestinationDatabase class.
   * @param module The module to export to.
   */
  void ExportDestination(pybind11::module& module);

  /**
   * Exports the ExchangeRate class.
   * @param module The module to export to.
   */
  void ExportExchangeRate(pybind11::module& module);

  /**
   * Exports the ExchangeRateTable class.
   * @param module The module to export to.
   */
  void ExportExchangeRateTable(pybind11::module& module);

  /**
   * Exports the MarketDatabase class.
   * @param module The module to export to.
   */
  void ExportMarket(pybind11::module& module);

  /**
   * Exports the MarketQuote struct.
   * @param module The module to export to.
   */
  void ExportMarketQuote(pybind11::module& module);

  /**
   * Exports the Money class.
   * @param module The module to export to.
   */
  void ExportMoney(pybind11::module& module);

  /**
   * Exports the OrderImbalance class.
   * @param module The module to export to.
   */
  void ExportOrderImbalance(pybind11::module& module);

  /**
   * Exports the OrderStatus enum.
   * @param module The module to export to.
   */
  void ExportOrderStatus(pybind11::module& module);

  /**
   * Exports the OrderType enum.
   * @param module The module to export to.
   */
  void ExportOrderType(pybind11::module& module);

  /**
   * Exports the Quantity class.
   * @param module The module to export to.
   */
  void ExportQuantity(pybind11::module& module);

  /**
   * Exports the Quote class.
   * @param module The module to export to.
   */
  void ExportQuote(pybind11::module& module);

  /**
   * Exports the Region class.
   * @param module The module to export to.
   */
  void ExportRegion(pybind11::module& module);

  /**
   * Exports the Security class.
   * @param module The module to export to.
   */
  void ExportSecurity(pybind11::module& module);

  /**
   * Exports the SecurityInfo class.
   * @param module The module to export to.
   */
  void ExportSecurityInfo(pybind11::module& module);

  /**
   * Exports the SecurityTechnicals class.
   * @param module The module to export to.
   */
  void ExportSecurityTechnicals(pybind11::module& module);

  /**
   * Exports the Side enum.
   * @param module The module to export to.
   */
  void ExportSide(pybind11::module& module);

  /**
   * Exports the Tag class.
   * @param module The module to export to.
   */
  void ExportTag(pybind11::module& module);

  /**
   * Exports the TimeAndSale struct.
   * @param module The module to export to.
   */
  void ExportTimeAndSale(pybind11::module& module);

  /**
   * Exports the TimeInForce class.
   * @param module The module to export to.
   */
  void ExportTimeInForce(pybind11::module& module);

  /**
   * Exports the TradingSchedule class.
   * @param module The module to export to.
   */
  void ExportTradingSchedule(pybind11::module& module);
}

#endif
