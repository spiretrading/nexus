#ifndef NEXUS_PYTHON_DEFINITIONS_HPP
#define NEXUS_PYTHON_DEFINITIONS_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the Asset class.
   * @param module The module to export to.
   */
  void export_asset(pybind11::module& module);

  /**
   * Exports the BboQuote struct.
   * @param module The module to export to.
   */
  void export_bbo_quote(pybind11::module& module);

  /**
   * Exports the BookQuote struct.
   * @param module The module to export to.
   */
  void export_book_quote(pybind11::module& module);

  /**
   * Exports the CountryDatabase class.
   * @param module The module to export to.
   */
  void export_country(pybind11::module& module);

  /**
   * Exports the CurrencyDatabase class.
   * @param module The module to export to.
   */
  void export_currency(pybind11::module& module);

  /**
   * Exports the CurrencyPair class.
   * @param module The module to export to.
   */
  void export_currency_pair(pybind11::module& module);

  /**
   * Exports the DefaultCountries namespace.
   * @param module The module to export to.
   */
  void export_default_countries(pybind11::module& module);

  /**
   * Exports the DefaultCurrencies namespace.
   * @param module The module to export to.
   */
  void export_default_currencies(pybind11::module& module);

  /**
   * Exports the DefaultDestinations namespace.
   * @param module The module to export to.
   */
  void export_default_destinations(pybind11::module& module);

  /**
   * Exports the DefaultVenues namespace.
   * @param module The module to export to.
   */
  void export_default_venues(pybind11::module& module);

  /**
   * Exports the Definitions classes.
   * @param module The module to export to.
   */
  void export_definitions(pybind11::module& module);

  /**
   * Exports the DestinationDatabase class.
   * @param module The module to export to.
   */
  void export_destination(pybind11::module& module);

  /**
   * Exports the ExchangeRate class.
   * @param module The module to export to.
   */
  void export_exchange_rate(pybind11::module& module);

  /**
   * Exports the ExchangeRateTable class.
   * @param module The module to export to.
   */
  void export_exchange_rate_table(pybind11::module& module);

  /**
   * Exports the Money class.
   * @param module The module to export to.
   */
  void export_money(pybind11::module& module);

  /**
   * Exports the OrderImbalance class.
   * @param module The module to export to.
   */
  void export_order_imbalance(pybind11::module& module);

  /**
   * Exports the OrderStatus enum.
   * @param module The module to export to.
   */
  void export_order_status(pybind11::module& module);

  /**
   * Exports the OrderType enum.
   * @param module The module to export to.
   */
  void export_order_type(pybind11::module& module);

  /**
   * Exports the Quantity class.
   * @param module The module to export to.
   */
  void export_quantity(pybind11::module& module);

  /**
   * Exports the Quote class.
   * @param module The module to export to.
   */
  void export_quote(pybind11::module& module);

  /**
   * Exports the Region class.
   * @param module The module to export to.
   */
  void export_region(pybind11::module& module);

  /**
   * Exports the RegionMap class.
   * @param module The module to export to.
   */
  void export_region_map(pybind11::module& module);

  /**
   * Exports the Security class.
   * @param module The module to export to.
   */
  void export_security(pybind11::module& module);

  /**
   * Exports the SecurityInfo class.
   * @param module The module to export to.
   */
  void export_security_info(pybind11::module& module);

  /**
   * Exports the SecurityTechnicals class.
   * @param module The module to export to.
   */
  void export_security_technicals(pybind11::module& module);

  /**
   * Exports the Side enum.
   * @param module The module to export to.
   */
  void export_side(pybind11::module& module);

  /**
   * Exports the Tag class.
   * @param module The module to export to.
   */
  void export_tag(pybind11::module& module);

  /**
   * Exports the TimeAndSale struct.
   * @param module The module to export to.
   */
  void export_time_and_sale(pybind11::module& module);

  /**
   * Exports the TimeInForce class.
   * @param module The module to export to.
   */
  void export_time_in_force(pybind11::module& module);

  /**
   * Exports the TradingSchedule class.
   * @param module The module to export to.
   */
  void export_trading_schedule(pybind11::module& module);

  /**
   * Exports the VenueDatabase class.
   * @param module The module to export to.
   */
  void export_venue(pybind11::module& module);
}

#endif
