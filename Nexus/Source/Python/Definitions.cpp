#include "Nexus/Python/Definitions.hpp"
#include <boost/lexical_cast.hpp>
#include <Beam/Python/Beam.hpp>
#include <pybind11/operators.h>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/CurrencyPair.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  template<typename T>
  auto BindIntegerPrecision(T (*function)(T, int)) {
    return [=] (T object) {
      return function(std::move(object), 0);
    };
  }
}

void Nexus::Python::ExportBboQuote(module& module) {
  class_<BboQuote>(module, "BboQuote").
    def(init()).
    def(init<const Quote&, const Quote&, ptime>()).
    def(init<const BboQuote&>()).
    def_readwrite("bid", &BboQuote::m_bid).
    def_readwrite("ask", &BboQuote::m_ask).
    def_readwrite("timestamp", &BboQuote::m_timestamp).
    def("__str__", &lexical_cast<std::string, BboQuote>).
    def(self == self).
    def(self != self);
  ExportQueueSuite<BboQuote>(module, "BboQuote");
  ExportQueueSuite<SequencedBboQuote>(module, "SequencedBboQuote");
}

void Nexus::Python::ExportBookQuote(module& module) {
  class_<BookQuote>(module, "BookQuote").
    def(init()).
    def(init<std::string, bool, MarketCode, const Quote&, ptime>()).
    def(init<const BookQuote&>()).
    def_readwrite("mpid", &BookQuote::m_mpid).
    def_readwrite("is_primary_mpid", &BookQuote::m_isPrimaryMpid).
    def_readwrite("market", &BookQuote::m_market).
    def_readwrite("quote", &BookQuote::m_quote).
    def_readwrite("timestamp", &BookQuote::m_timestamp).
    def("__str__", &lexical_cast<std::string, BookQuote>).
    def(self == self).
    def(self != self);
  ExportQueueSuite<BookQuote>(module, "BookQuote");
  ExportQueueSuite<SequencedBookQuote>(module, "SequencedBookQuote");
  module.def("book_quote_listing_comparator", &BookQuoteListingComparator);
}

void Nexus::Python::ExportCountry(module& module) {
  class_<CountryCode>(module, "CountryCode").
    def(init()).
    def(init<int>()).
    def_readonly_static("NONE", &CountryCode::NONE).
    def("__str__", &lexical_cast<std::string, CountryCode>).
    def("__hash__", [] (CountryCode self) {
      return std::hash<CountryCode>()(self);
    }).
    def(self == self).
    def(self != self).
    def(self < self);
  module.def("parse_country_code",
    static_cast<CountryCode (*)(const std::string&, const CountryDatabase&)>(
      &ParseCountryCode));
  module.def("parse_country_code",
    static_cast<CountryCode (*)(const std::string&)>(&ParseCountryCode));
  auto outer = class_<CountryDatabase>(module, "CountryDatabase").
    def(init()).
    def(init<const CountryDatabase&>()).
    def_property_readonly("entries", &CountryDatabase::GetEntries).
    def("from_code", &CountryDatabase::FromCode).
    def("from_name", &CountryDatabase::FromName).
    def("from_two_letter_code", &CountryDatabase::FromTwoLetterCode).
    def("from_three_letter_code", &CountryDatabase::FromThreeLetterCode).
    def("add", &CountryDatabase::Add).
    def("delete", &CountryDatabase::Delete);
  class_<CountryDatabase::Entry>(outer, "Entry").
    def(init<const CountryDatabase::Entry&>()).
    def_readwrite("code", &CountryDatabase::Entry::m_code).
    def_readwrite("name", &CountryDatabase::Entry::m_name).
    def_readwrite("two_letter_code", &CountryDatabase::Entry::m_twoLetterCode).
    def_readwrite("three_letter_code",
      &CountryDatabase::Entry::m_threeLetterCode);
}

void Nexus::Python::ExportCurrency(module& module) {
  class_<CurrencyId>(module, "CurrencyId").
    def(init()).
    def(init<int>()).
    def_readonly_static("NONE", &CurrencyId::NONE).
    def("__str__", &lexical_cast<std::string, CurrencyId>).
    def("__hash__", [] (CurrencyId self) {
      return std::hash<CurrencyId>()(self);
    }).
    def(self == self).
    def(self != self).
    def(self < self);
  auto outer = class_<CurrencyDatabase>(module, "CurrencyDatabase").
    def(init()).
    def(init<const CurrencyDatabase&>()).
    def_property_readonly("entries", &CurrencyDatabase::GetEntries).
    def("from_id", &CurrencyDatabase::FromId).
    def("from_code", &CurrencyDatabase::FromCode).
    def("add", &CurrencyDatabase::Add).
    def("delete", &CurrencyDatabase::Delete);
  class_<CurrencyDatabase::Entry>(outer, "Entry").
    def(init<const CurrencyDatabase::Entry&>()).
    def_readwrite("id", &CurrencyDatabase::Entry::m_id).
    def_readwrite("code", &CurrencyDatabase::Entry::m_code).
    def_readwrite("sign", &CurrencyDatabase::Entry::m_sign);
  module.def("parse_currency", static_cast<
    CurrencyId (*)(const std::string&, const CurrencyDatabase&)>(
      &ParseCurrency));
  module.def("parse_currency", static_cast<
    CurrencyId (*)(const std::string&)>(&ParseCurrency));
}

void Nexus::Python::ExportCurrencyPair(module& module) {
  class_<CurrencyPair>(module, "CurrencyPair").
    def(init()).
    def(init<CurrencyId, CurrencyId>()).
    def(init<const CurrencyPair&>()).
    def_readwrite("base", &CurrencyPair::m_base).
    def_readwrite("counter", &CurrencyPair::m_counter).
    def(self == self).
    def(self != self).
    def(self < self);
  module.def("parse_currency_pair",
    static_cast<CurrencyPair (*)(const std::string&, const CurrencyDatabase&)>(
      &ParseCurrencyPair));
  module.def("parse_currency_pair",
    static_cast<CurrencyPair (*)(const std::string&)>(&ParseCurrencyPair));
  module.def("invert",
    static_cast<CurrencyPair (*)(const CurrencyPair&)>(&Invert));
}

void Nexus::Python::ExportDefaultCountries(module& module) {
  auto submodule = module.def_submodule("default_countries");
  submodule.add_object("AU", cast(DefaultCountries::AU()));
  submodule.add_object("BR", cast(DefaultCountries::BR()));
  submodule.add_object("CA", cast(DefaultCountries::CA()));
  submodule.add_object("CN", cast(DefaultCountries::CN()));
  submodule.add_object("HK", cast(DefaultCountries::HK()));
  submodule.add_object("JP", cast(DefaultCountries::JP()));
  submodule.add_object("US", cast(DefaultCountries::US()));
}

void Nexus::Python::ExportDefaultCurrencies(module& module) {
  auto submodule = module.def_submodule("default_currencies");
  submodule.add_object("AUD", cast(DefaultCurrencies::AUD()));
  submodule.add_object("CAD", cast(DefaultCurrencies::CAD()));
  submodule.add_object("HKD", cast(DefaultCurrencies::HKD()));
  submodule.add_object("JPY", cast(DefaultCurrencies::JPY()));
  submodule.add_object("USD", cast(DefaultCurrencies::USD()));
  submodule.add_object("XBT", cast(DefaultCurrencies::XBT()));
}

void Nexus::Python::ExportDefaultDestinations(module& module) {
  auto submodule = module.def_submodule("default_destinations");
  submodule.add_object("MOE", cast(DefaultDestinations::MOE()));
  submodule.add_object("ASXT", cast(DefaultDestinations::ASXT()));
  submodule.add_object("CXA", cast(DefaultDestinations::CXA()));
  submodule.add_object("ARCA", cast(DefaultDestinations::ARCA()));
  submodule.add_object("BATS", cast(DefaultDestinations::BATS()));
  submodule.add_object("NYSE", cast(DefaultDestinations::NYSE()));
  submodule.add_object("NASDAQ", cast(DefaultDestinations::NASDAQ()));
  submodule.add_object("HKEX", cast(DefaultDestinations::HKEX()));
  submodule.add_object("ALPHA", cast(DefaultDestinations::ALPHA()));
  submodule.add_object("CHIX", cast(DefaultDestinations::CHIX()));
  submodule.add_object("CSE", cast(DefaultDestinations::CSE()));
  submodule.add_object("CSE2", cast(DefaultDestinations::CSE2()));
  submodule.add_object("CX2", cast(DefaultDestinations::CX2()));
  submodule.add_object("LYNX", cast(DefaultDestinations::LYNX()));
  submodule.add_object("MATNLP", cast(DefaultDestinations::MATNLP()));
  submodule.add_object("MATNMF", cast(DefaultDestinations::MATNMF()));
  submodule.add_object("NEOE", cast(DefaultDestinations::NEOE()));
  submodule.add_object("OMEGA", cast(DefaultDestinations::OMEGA()));
  submodule.add_object("PURE", cast(DefaultDestinations::PURE()));
  submodule.add_object("TSX", cast(DefaultDestinations::TSX()));
  submodule.add_object("OSE", cast(DefaultDestinations::OSE()));
  submodule.add_object("TSE", cast(DefaultDestinations::TSE()));
}

void Nexus::Python::ExportDefaultMarkets(module& module) {
  auto submodule = module.def_submodule("default_markets");
  submodule.add_object("HKEX", cast(DefaultMarkets::HKEX()));
  submodule.add_object("ASX", cast(DefaultMarkets::ASX()));
  submodule.add_object("CXA", cast(DefaultMarkets::CXA()));
  submodule.add_object("ASEX", cast(DefaultMarkets::ASEX()));
  submodule.add_object("ARCX", cast(DefaultMarkets::ARCX()));
  submodule.add_object("BATS", cast(DefaultMarkets::BATS()));
  submodule.add_object("BATY", cast(DefaultMarkets::BATY()));
  submodule.add_object("BOSX", cast(DefaultMarkets::BOSX()));
  submodule.add_object("CBOE", cast(DefaultMarkets::CBOE()));
  submodule.add_object("CSE", cast(DefaultMarkets::CSE()));
  submodule.add_object("CSE2", cast(DefaultMarkets::CSE2()));
  submodule.add_object("NSEX", cast(DefaultMarkets::NSEX()));
  submodule.add_object("ADFX", cast(DefaultMarkets::ADFX()));
  submodule.add_object("ISE", cast(DefaultMarkets::ISE()));
  submodule.add_object("EDGA", cast(DefaultMarkets::EDGA()));
  submodule.add_object("EDGX", cast(DefaultMarkets::EDGX()));
  submodule.add_object("PHLX", cast(DefaultMarkets::PHLX()));
  submodule.add_object("CHIC", cast(DefaultMarkets::CHIC()));
  submodule.add_object("LYNX", cast(DefaultMarkets::LYNX()));
  submodule.add_object("NASDAQ", cast(DefaultMarkets::NASDAQ()));
  submodule.add_object("NYSE", cast(DefaultMarkets::NYSE()));
  submodule.add_object("MATN", cast(DefaultMarkets::MATN()));
  submodule.add_object("NEOE", cast(DefaultMarkets::NEOE()));
  submodule.add_object("OMGA", cast(DefaultMarkets::OMGA()));
  submodule.add_object("PURE", cast(DefaultMarkets::PURE()));
  submodule.add_object("TSX", cast(DefaultMarkets::TSX()));
  submodule.add_object("TSXV", cast(DefaultMarkets::TSXV()));
  submodule.add_object("XATS", cast(DefaultMarkets::XATS()));
  submodule.add_object("XCX2", cast(DefaultMarkets::XCX2()));
  submodule.add_object("XFKA", cast(DefaultMarkets::XFKA()));
  submodule.add_object("TSE", cast(DefaultMarkets::TSE()));
  submodule.add_object("OSE", cast(DefaultMarkets::OSE()));
  submodule.add_object("NSE", cast(DefaultMarkets::NSE()));
  submodule.add_object("SSE", cast(DefaultMarkets::SSE()));
  submodule.add_object("CHIJ", cast(DefaultMarkets::CHIJ()));
}

void Nexus::Python::ExportDefinitions(module& module) {
  ExportBboQuote(module);
  ExportBookQuote(module);
  ExportCountry(module);
  ExportCurrency(module);
  ExportCurrencyPair(module);
  ExportDefaultCountries(module);
  ExportDefaultCurrencies(module);
  ExportDefaultDestinations(module);
  ExportDefaultMarkets(module);
  ExportDestination(module);
  ExportExchangeRate(module);
  ExportExchangeRateTable(module);
  ExportMarket(module);
  ExportMarketQuote(module);
  ExportMoney(module);
  ExportOrderImbalance(module);
  ExportOrderStatus(module);
  ExportOrderType(module);
  ExportQuantity(module);
  ExportQuote(module);
  ExportRegion(module);
  ExportSecurity(module);
  ExportSecurityInfo(module);
  ExportSecurityTechnicals(module);
  ExportSide(module);
  ExportTag(module);
  ExportTimeAndSale(module);
  ExportTimeInForce(module);
  ExportTradingSchedule(module);
}

void Nexus::Python::ExportDestination(module& module) {
  auto outer = class_<DestinationDatabase>(module, "DestinationDatabase").
    def(init()).
    def(init<const DestinationDatabase&>()).
    def("from_id", &DestinationDatabase::FromId).
    def("get_preferred_destination",
      &DestinationDatabase::GetPreferredDestination).
    def("select_entry",
      [] (DestinationDatabase& self, const object& predicate) {
        return self.SelectEntry(predicate);
      }).
    def("select_entries",
      [] (DestinationDatabase& self, const object& predicate) {
        return self.SelectEntries(predicate);
      }).
    def_property("manual_order_entry_destination",
      &DestinationDatabase::GetManualOrderEntryDestination,
      &DestinationDatabase::SetManualOrderEntryDestination).
    def("add", &DestinationDatabase::Add).
    def("set_preferred_destination",
      &DestinationDatabase::SetPreferredDesintation).
    def("delete", &DestinationDatabase::Delete).
    def("delete_preferred_destination",
      &DestinationDatabase::DeletePreferredDestination);
  class_<DestinationDatabase::Entry>(outer, "Entry").
    def(init<const DestinationDatabase::Entry&>()).
    def_readwrite("id", &DestinationDatabase::Entry::m_id).
    def_readwrite("markets", &DestinationDatabase::Entry::m_markets).
    def_readwrite("description", &DestinationDatabase::Entry::m_description);
}

void Nexus::Python::ExportExchangeRate(module& module) {
  class_<ExchangeRate>(module, "ExchangeRate").
    def(init()).
    def(init<const CurrencyPair&, const rational<int>&>()).
    def(init<const ExchangeRate&>()).
    def_readwrite("pair", &ExchangeRate::m_pair).
    def_readwrite("rate", &ExchangeRate::m_rate);
  module.def("invert", static_cast<ExchangeRate (*)(const ExchangeRate&)>(
    &Invert));
  module.def("convert", &Convert);
}

void Nexus::Python::ExportExchangeRateTable(module& module) {
  class_<ExchangeRateTable>(module, "ExchangeRateTable").
    def(init()).
    def("find", &ExchangeRateTable::Find).
    def("convert", &ExchangeRateTable::Convert).
    def("add", &ExchangeRateTable::Add);
}

void Nexus::Python::ExportMarket(module& module) {
  auto outer = class_<MarketDatabase>(module, "MarketDatabase").
    def(init()).
    def(init<const MarketDatabase&>()).
    def_property_readonly("entries", &MarketDatabase::GetEntries).
    def("from_code", &MarketDatabase::FromCode).
    def("from_display_name", &MarketDatabase::FromDisplayName).
    def("from_country", &MarketDatabase::FromCountry).
    def("add", &MarketDatabase::Add).
    def("delete", &MarketDatabase::Delete);
  class_<MarketDatabase::Entry>(outer, "Entry").
    def(init<const MarketDatabase::Entry&>()).
    def_readwrite("code", &MarketDatabase::Entry::m_code).
    def_readwrite("country_code", &MarketDatabase::Entry::m_countryCode).
    def_readwrite("time_zone", &MarketDatabase::Entry::m_timeZone).
    def_readwrite("currency", &MarketDatabase::Entry::m_currency).
    def_readwrite("description", &MarketDatabase::Entry::m_description).
    def_readwrite("display_name", &MarketDatabase::Entry::m_displayName).
    def(self == self).
    def(self != self).
    def("__str__", &lexical_cast<std::string, MarketDatabase::Entry>);
  module.def("parse_market_code", static_cast<
    MarketCode (*)(const std::string&, const MarketDatabase&)>(
      &ParseMarketCode));
  module.def("parse_market_code", static_cast<
    MarketCode (*)(const std::string&)>(&ParseMarketCode));
  module.def("parse_market_entry", &ParseMarketEntry);
}

void Nexus::Python::ExportMarketQuote(module& module) {
  class_<MarketQuote>(module, "MarketQuote").
    def(init()).
    def(init<MarketCode, const Quote&, const Quote&, ptime>()).
    def(init<const MarketQuote&>()).
    def_readwrite("market", &MarketQuote::m_market).
    def_readwrite("bid", &MarketQuote::m_bid).
    def_readwrite("ask", &MarketQuote::m_ask).
    def_readwrite("timestamp", &MarketQuote::m_timestamp).
    def("__str__", &lexical_cast<std::string, MarketQuote>).
    def(self == self).
    def(self != self);
  ExportQueueSuite<MarketQuote>(module, "MarketQuote");
  ExportQueueSuite<SequencedMarketQuote>(module, "SequencedMarketQuote");
}

void Nexus::Python::ExportMoney(module& module) {
  class_<Money>(module, "Money").
    def(init()).
    def(init<const Money&>()).
    def(init<double>()).
    def(init<Quantity>()).
    def_readonly_static("ZERO", &Money::ZERO).
    def_readonly_static("ONE", &Money::ONE).
    def_readonly_static("CENT", &Money::CENT).
    def_readonly_static("BIP", &Money::BIP).
    def_static("from_value",
      static_cast<boost::optional<Money> (*)(const std::string&)>(
      &Money::FromValue)).
    def("__str__", &lexical_cast<std::string, Money>).
    def("__abs__", static_cast<Money (*)(Money)>(&Abs)).
    def("__floor__", BindIntegerPrecision(
      static_cast<Money (*)(Money, int)>(&Floor))).
    def("__hash__", [] (Money self) {
      return std::hash<Money>()(self);
    }).
    def("__ceil__", BindIntegerPrecision(
      static_cast<Money (*)(Money, int)>(&Ceil))).
    def("__trunc__", BindIntegerPrecision(
      static_cast<Money (*)(Money, int)>(&Truncate))).
    def("__round__", BindIntegerPrecision(
      static_cast<Money (*)(Money, int)>(&Round))).
    def("__float__",
      [] (Money self) {
        return static_cast<double>(self);
      }).
    def(self < self).
    def(self <= self).
    def(self == self).
    def(self != self).
    def(self >= self).
    def(self > self).
    def(self + self).
    def(self - self).
    def(self / self).
    def(-self).
    def(Quantity() * self).
    def(double() * self).
    def(int() * self).
    def(self / Quantity()).
    def(self / double()).
    def(self / int());
  implicitly_convertible<std::int32_t, Money>();
  implicitly_convertible<std::int64_t, Money>();
  implicitly_convertible<double, Money>();
  module.def("floor", static_cast<Money (*)(Money, int)>(&Floor));
  module.def("ceil", static_cast<Money (*)(Money, int)>(&Ceil));
  module.def("truncate", static_cast<Money (*)(Money, int)>(&Truncate));
  module.def("round", static_cast<Money (*)(Money, int)>(&Round));
}

void Nexus::Python::ExportOrderImbalance(module& module) {
  class_<OrderImbalance>(module, "OrderImbalance").
    def(init()).
    def(init<Security, Side, Quantity, Money, ptime>()).
    def(init<const OrderImbalance&>()).
    def_readwrite("security", &OrderImbalance::m_security).
    def_readwrite("side", &OrderImbalance::m_side).
    def_readwrite("size", &OrderImbalance::m_size).
    def_readwrite("reference_price", &OrderImbalance::m_referencePrice).
    def_readwrite("timestamp", &OrderImbalance::m_timestamp).
    def("__str__", &lexical_cast<std::string, OrderImbalance>).
    def(self == self).
    def(self != self);
  ExportQueueSuite<OrderImbalance>(module, "OrderImbalance");
  ExportQueueSuite<SequencedOrderImbalance>(module, "SequencedOrderImbalance");
}

void Nexus::Python::ExportOrderStatus(module& module) {
  enum_<OrderStatus::Type>(module, "OrderStatus").
    value("NONE", OrderStatus::NONE).
    value("PENDING_NEW", OrderStatus::PENDING_NEW).
    value("REJECTED", OrderStatus::REJECTED).
    value("NEW", OrderStatus::NEW).
    value("PARTIALLY_FILLED", OrderStatus::PARTIALLY_FILLED).
    value("EXPIRED", OrderStatus::EXPIRED).
    value("CANCELED", OrderStatus::CANCELED).
    value("SUSPENDED", OrderStatus::SUSPENDED).
    value("STOPPED", OrderStatus::STOPPED).
    value("FILLED", OrderStatus::FILLED).
    value("DONE_FOR_DAY", OrderStatus::DONE_FOR_DAY).
    value("PENDING_CANCEL", OrderStatus::PENDING_CANCEL).
    value("CANCEL_REJECT", OrderStatus::CANCEL_REJECT);
  module.def("is_terminal", &IsTerminal);
}

void Nexus::Python::ExportOrderType(module& module) {
  enum_<OrderType::Type>(module, "OrderType").
    value("NONE", OrderType::NONE).
    value("MARKET", OrderType::MARKET).
    value("LIMIT", OrderType::LIMIT).
    value("PEGGED", OrderType::PEGGED).
    value("STOP", OrderType::STOP);
}

void Nexus::Python::ExportQuantity(module& module) {
  class_<Quantity>(module, "Quantity").
    def(init()).
    def(init<int>()).
    def(init<double>()).
    def(init<const Quantity&>()).
    def_static("from_value",
      static_cast<optional<Quantity> (*)(const std::string&)>(
        &Quantity::FromValue)).
    def("__str__", &lexical_cast<std::string, Quantity>).
    def("__abs__", static_cast<Quantity (*)(Quantity)>(&Abs)).
    def("__floor__", BindIntegerPrecision(
      static_cast<Quantity (*)(Quantity, int)>(&Floor))).
    def("__ceil__", BindIntegerPrecision(
      static_cast<Quantity (*)(Quantity, int)>(&Ceil))).
    def("__hash__", [] (Quantity self) {
      return std::hash<Quantity>()(self);
    }).
    def("__trunc__", BindIntegerPrecision(
      static_cast<Quantity (*)(Quantity, int)>(&Truncate))).
    def("__round__", BindIntegerPrecision(
      static_cast<Quantity (*)(Quantity, int)>(&Round))).
    def("__int__",
      [] (Quantity self) {
        return static_cast<int>(self);
      }).
    def("__float__",
      [] (Quantity self) {
        return static_cast<double>(self);
      }).
    def(self < self).
    def(self < double()).
    def(double() < self).
    def(self < int()).
    def(int() < self).
    def(self <= self).
    def(self <= double()).
    def(double() <= self).
    def(self <= int()).
    def(int() <= self).
    def(self == self).
    def(self == double()).
    def(double() == self).
    def(self == int()).
    def(int() == self).
    def(self != self).
    def(self != double()).
    def(double() != self).
    def(self != int()).
    def(int() != self).
    def(self >= self).
    def(self >= double()).
    def(double() >= self).
    def(self >= int()).
    def(int() >= self).
    def(self > self).
    def(self > double()).
    def(double() > self).
    def(self > int()).
    def(int() > self).
    def(self + self).
    def(self + double()).
    def(double() + self).
    def(self + int()).
    def(int() + self).
    def(self - self).
    def(self - double()).
    def(double() - self).
    def(self - int()).
    def(int() - self).
    def(self * self).
    def(self * double()).
    def(double() * self).
    def(self * int()).
    def(int() * self).
    def(self / self).
    def(self / double()).
    def(double() / self).
    def(self / int()).
    def(int() / self).
    def(-self);
  implicitly_convertible<std::int32_t, Quantity>();
  implicitly_convertible<std::int64_t, Quantity>();
  implicitly_convertible<double, Quantity>();
  module.def("floor", static_cast<Quantity (*)(Quantity, int)>(&Floor));
  module.def("ceil", static_cast<Quantity (*)(Quantity, int)>(&Ceil));
  module.def("truncate", static_cast<Quantity (*)(Quantity, int)>(&Truncate));
  module.def("round", static_cast<Quantity (*)(Quantity, int)>(&Round));
}

void Nexus::Python::ExportQuote(module& module) {
  class_<Quote>(module, "Quote").
    def(init()).
    def(init<Money, Quantity, Side>()).
    def(init<const Quote&>()).
    def_readwrite("price", &Quote::m_price).
    def_readwrite("size", &Quote::m_size).
    def_readwrite("side", &Quote::m_side).
    def("__str__", &lexical_cast<std::string, Quote>).
    def(self == self).
    def(self != self);
}

void Nexus::Python::ExportRegion(module& module) {
  class_<Region>(module, "Region").
    def_property_readonly_static("GLOBAL",
      [] (const object&) { return Region::Global(); }).
    def_static("make_global_region",
      [] (std::string name) {
        return Region::Global(std::move(name));
      }).
    def(init()).
    def(init<std::string>()).
    def(init<CountryCode>()).
    def(init<MarketCode, CountryCode>()).
    def(init<const MarketDatabase::Entry&>()).
    def(init<Security>()).
    def_property("name", &Region::GetName, &Region::SetName).
    def_property_readonly("is_global", &Region::IsGlobal).
    def_property_readonly("is_empty", &Region::IsEmpty).
    def_property_readonly("countries", &Region::GetCountries).
    def_property_readonly("markets", &Region::GetMarkets).
    def_property_readonly("securities", &Region::GetSecurities).
    def("contains", &Region::Contains).
    def("__hash__", static_cast<std::size_t (*)(const Region&)>(hash_value)).
    def("__str__", &lexical_cast<std::string, Region>).
    def(self + self).
    def(self += self).
    def(self < self).
    def(self <= self).
    def(self == self).
    def(self != self).
    def(self >= self).
    def(self > self);
}

void Nexus::Python::ExportSecurity(module& module) {
  class_<Security>(module, "Security").
    def(init()).
    def(init<const std::string&, MarketCode, CountryCode>()).
    def(init<const std::string&, CountryCode>()).
    def(init<const Security&>()).
    def(self < self).
    def(self == self).
    def(self != self).
    def("__hash__", static_cast<std::size_t (*)(const Security&)>(hash_value)).
    def("__str__", static_cast<std::string (*)(const Security&)>(&ToString)).
    def_property_readonly("symbol", &Security::GetSymbol).
    def_property_readonly("market", &Security::GetMarket).
    def_property_readonly("country", &Security::GetCountry);
  module.def("to_string",
    static_cast<std::string (*)(const Security&, const MarketDatabase&)>(
      &ToString));
  module.def("parse_security",
    static_cast<Security (*)(const std::string&, const MarketDatabase&)>(
      &ParseSecurity));
  module.def("parse_security",
    static_cast<Security (*)(const std::string&)>(&ParseSecurity));
}

void Nexus::Python::ExportSecurityInfo(module& module) {
  class_<SecurityInfo>(module, "SecurityInfo").
    def(init()).
    def(init<const SecurityInfo&>()).
    def(init<const Security&, std::string, std::string, Quantity>()).
    def_readwrite("security", &SecurityInfo::m_security).
    def_readwrite("name", &SecurityInfo::m_name).
    def_readwrite("sector", &SecurityInfo::m_sector).
    def_readwrite("board_lot", &SecurityInfo::m_boardLot).
    def(self == self).
    def(self != self).
    def("__hash__",
      [] (const SecurityInfo& self) {
        return std::hash<SecurityInfo>()(self);
      }).
    def("__str__", &lexical_cast<std::string, SecurityInfo>);
}

void Nexus::Python::ExportSecurityTechnicals(module& module) {
  class_<SecurityTechnicals>(module, "SecurityTechnicals").
    def(init()).
    def(init<const SecurityTechnicals&>()).
    def_readwrite("volume", &SecurityTechnicals::m_volume).
    def_readwrite("high", &SecurityTechnicals::m_high).
    def_readwrite("low", &SecurityTechnicals::m_low).
    def_readwrite("open", &SecurityTechnicals::m_open).
    def_readwrite("close", &SecurityTechnicals::m_close);
}

void Nexus::Python::ExportSide(module& module) {
  enum_<Side::Type>(module, "Side").
    value("NONE", Side::NONE).
    value("ASK", Side::ASK).
    value("BID", Side::BID);
  module.def("pick",
    [] (Side side, object a, object b) {
      return Pick(side, a, b);
    });
  module.def("get_direction", &GetDirection);
  module.def("get_side", &GetSide);
  module.def("get_opposite", &GetOpposite);
}

void Nexus::Python::ExportTag(module& module) {
  class_<Tag>(module, "Tag").
    def(init()).
    def(init<int, const Tag::Type&>()).
    def(init<const Tag&>()).
    def_property_readonly("key", &Tag::GetKey).
    def_property_readonly("value", &Tag::GetValue).
    def("__str__", &lexical_cast<std::string, Tag>).
    def(self == self);
}

void Nexus::Python::ExportTimeAndSale(module& module) {
  auto outer = class_<TimeAndSale>(module, "TimeAndSale").
    def(init()).
    def(init<ptime, Money, Quantity, TimeAndSale::Condition, std::string>()).
    def(init<const TimeAndSale&>()).
    def_readwrite("timestamp", &TimeAndSale::m_timestamp).
    def_readwrite("price", &TimeAndSale::m_price).
    def_readwrite("size", &TimeAndSale::m_size).
    def_readwrite("condition", &TimeAndSale::m_condition).
    def_readwrite("market_center", &TimeAndSale::m_marketCenter).
    def("__str__", &lexical_cast<std::string, TimeAndSale>).
    def(self == self).
    def(self != self);
  {
    auto inner = class_<TimeAndSale::Condition>(outer, "Condition").
      def(init()).
      def(init<const TimeAndSale::Condition&>()).
      def_readwrite("type", &TimeAndSale::Condition::m_type).
      def_readwrite("code", &TimeAndSale::Condition::m_code).
      def("__str__", &lexical_cast<std::string, TimeAndSale::Condition>).
      def(self == self).
      def(self != self);
    enum_<TimeAndSale::Condition::Type::Type>(inner, "Type").
      value("NONE", TimeAndSale::Condition::Type::NONE).
      value("REGULAR", TimeAndSale::Condition::Type::REGULAR).
      value("OPEN", TimeAndSale::Condition::Type::OPEN).
      value("CLOSE", TimeAndSale::Condition::Type::CLOSE);
  }
  ExportQueueSuite<TimeAndSale>(module, "TimeAndSale");
  ExportQueueSuite<SequencedTimeAndSale>(module, "SequencedTimeAndSale");
}

void Nexus::Python::ExportTimeInForce(module& module) {
  auto outer = class_<TimeInForce>(module, "TimeInForce").
    def(init()).
    def(init<TimeInForce::Type>()).
    def(init<TimeInForce::Type, ptime>()).
    def(init<const TimeInForce&>()).
    def_property_readonly("type", &TimeInForce::GetType).
    def_property_readonly("expiry", &TimeInForce::GetExpiry).
    def("__hash__", [] (const TimeInForce& self) {
      return std::hash<TimeInForce>()(self);
    }).
    def("__str__", &lexical_cast<std::string, TimeInForce>).
    def(self == self).
    def(self != self);
  enum_<TimeInForce::Type::Type>(outer, "Type").
    value("NONE", TimeInForce::Type::NONE).
    value("DAY", TimeInForce::Type::DAY).
    value("GTC", TimeInForce::Type::GTC).
    value("OPG", TimeInForce::Type::OPG).
    value("MOC", TimeInForce::Type::MOC).
    value("IOC", TimeInForce::Type::IOC).
    value("FOK", TimeInForce::Type::FOK).
    value("GTX", TimeInForce::Type::GTX).
    value("GTD", TimeInForce::Type::GTD);
}

void Nexus::Python::ExportTradingSchedule(module& module) {
  auto outer = class_<TradingSchedule>(module, "TradingSchedule").
    def(init<std::vector<TradingSchedule::Rule>>()).
    def("find", [] (TradingSchedule& self, date date, MarketCode market) {
      return self.Find(date, market);
    }).
    def("find", [] (TradingSchedule& self, date date, MarketCode market,
        const object& f) {
      return self.Find(date, market, f);
    });
  class_<TradingSchedule::Event>(outer, "Event").
    def_readwrite("code", &TradingSchedule::Event::m_code).
    def_readwrite("timestamp", &TradingSchedule::Event::m_timestamp).
    def("__str__", &lexical_cast<std::string, TradingSchedule::Event>).
    def(self == self).
    def(self != self);
  class_<TradingSchedule::Rule>(outer, "Rule").
    def_readwrite("markets", &TradingSchedule::Rule::m_markets).
    def_readwrite("weekdays", &TradingSchedule::Rule::m_weekdays).
    def_readwrite("days", &TradingSchedule::Rule::m_days).
    def_readwrite("months", &TradingSchedule::Rule::m_months).
    def_readwrite("years", &TradingSchedule::Rule::m_years).
    def_readwrite("events", &TradingSchedule::Rule::m_events).
    def(self == self).
    def(self != self);
  module.def("is_match",
    static_cast<bool (*)(MarketCode, date, const TradingSchedule::Rule&)>(
      &IsMatch));
}
