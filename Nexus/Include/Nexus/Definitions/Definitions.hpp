#ifndef NEXUS_DEFINITIONS_HPP
#define NEXUS_DEFINITIONS_HPP

namespace Nexus {
  struct BboQuote;
  struct BookQuote;
  struct BookQuotePair;
  class CountryCode;
  class CountryDatabase;
  class CurrencyDatabase;
  struct CurrencyId;
  struct CurrencyPair;
  class CurrencyPairNotFoundException;
  class DestinationDatabase;
  struct ExchangeRate;
  class ExchangeRateTable;
  enum class LiquidityFlag;
  class MarketDatabase;
  struct MarketQuote;
  template<typename MarketDataType> struct MarketWideData;
  class Money;
  struct OrderImbalance;
  struct Quote;
  class Region;
  template<typename T> class RegionMap;
  class Security;
  struct SecurityInfo;
  template<typename MarketDataType> struct SecurityMarketData;
  class SecuritySet;
  struct SecurityTechnicals;
  class Tag;
  struct TimeAndSale;
  class TimeInForce;
}

#endif
