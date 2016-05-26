#ifndef NEXUS_PYTHONDEFINITIONS_HPP
#define NEXUS_PYTHONDEFINITIONS_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the BboQuote struct.
  void ExportBboQuote();

  //! Exports the BookQuote struct.
  void ExportBookQuote();

  //! Exports the CountryDatabase class.
  void ExportCountry();

  //! Exports the CurrencyDatabase class.
  void ExportCurrency();

  //! Exports the CurrencyPair class.
  void ExportCurrencyPair();

  //! Exports the DefaultCountries namespace.
  void ExportDefaultCountries();

  //! Exports the DefaultCurrencies namespace.
  void ExportDefaultCurrencies();

  //! Exports the DefaultDestinations namespace.
  void ExportDefaultDestinations();

  //! Exports the DefaultMarkets namespace.
  void ExportDefaultMarkets();

  //! Exports the Definitions classes.
  void ExportDefinitions();

  //! Exports the DestinationDatabase class.
  void ExportDestination();

  //! Exports the ExchangeRate class.
  void ExportExchangeRate();

  //! Exports the ExchangeRateTable class.
  void ExportExchangeRateTable();

  //! Exports the MarketDatabase class.
  void ExportMarket();

  //! Exports the MarketQuote struct.
  void ExportMarketQuote();

  //! Exports the Money class.
  void ExportMoney();

  //! Exports the OrderImbalance class.
  void ExportOrderImbalance();

  //! Exports the OrderStatus enum.
  void ExportOrderStatus();

  //! Exports the OrderType enum.
  void ExportOrderType();

  //! Exports the Quote class.
  void ExportQuote();

  //! Exports the Security class.
  void ExportSecurity();

  //! Exports the SecurityTechnicals class.
  void ExportSecurityTechnicals();

  //! Exports the Side enum.
  void ExportSide();

  //! Exports the Tag class.
  void ExportTag();

  //! Exports the TimeAndSale struct.
  void ExportTimeAndSale();

  //! Exports the TimeInForce class.
  void ExportTimeInForce();
}
}

#endif
