#include "Nexus/Python/Definitions.hpp"
#include <boost/lexical_cast.hpp>
#include <Beam/Python/Beam.hpp>
#include <pybind11/operators.h>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/CurrencyPair.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/RegionMap.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

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
  auto bind_integer_precision(T (*function)(T, int)) {
    return [=] (T object) {
      return function(std::move(object), 0);
    };
  }
}

void Nexus::Python::export_bbo_quote(module& module) {
  class_<BboQuote>(module, "BboQuote").
    def(init()).
    def(init<const BboQuote&>()).
    def(init<Quote, Quote, ptime>()).
    def_readwrite("bid", &BboQuote::m_bid).
    def_readwrite("ask", &BboQuote::m_ask).
    def_readwrite("timestamp", &BboQuote::m_timestamp).
    def(self == self).
    def(self != self).
    def("__str__", &lexical_cast<std::string, BboQuote>);
  ExportQueueSuite<BboQuote>(module, "BboQuote");
  ExportQueueSuite<SequencedBboQuote>(module, "SequencedBboQuote");
}

void Nexus::Python::export_book_quote(module& module) {
  class_<BookQuote>(module, "BookQuote").
    def(init()).
    def(init<const BookQuote&>()).
    def(init<std::string, bool, Venue, Quote, ptime>()).
    def_readwrite("mpid", &BookQuote::m_mpid).
    def_readwrite("is_primary_mpid", &BookQuote::m_is_primary_mpid).
    def_readwrite("venue", &BookQuote::m_venue).
    def_readwrite("quote", &BookQuote::m_quote).
    def_readwrite("timestamp", &BookQuote::m_timestamp).
    def(self == self).
    def(self != self).
    def("__str__", &lexical_cast<std::string, BookQuote>);
  ExportQueueSuite<BookQuote>(module, "BookQuote");
  ExportQueueSuite<SequencedBookQuote>(module, "SequencedBookQuote");
  module.def("is_same_listing", &is_same_listing);
  module.def("listing_comparator", static_cast<bool (*)(
    const BookQuote&, const BookQuote&)>(&listing_comparator));
}

void Nexus::Python::export_country(module& module) {
  class_<CountryCode>(module, "CountryCode").
    def(init()).
    def(init<std::uint16_t>()).
    def_readonly_static("NONE", &CountryCode::NONE).
    def("__int__", [] (const CountryCode& self) {
      return static_cast<std::uint16_t>(self);
    }).
    def("__bool__", &CountryCode::operator bool).
    def(self == self).
    def(self != self).
    def(self < self).
    def("__str__", &lexical_cast<std::string, CountryCode>).
    def("__hash__", std::hash<CountryCode>());
  auto country_database = class_<CountryDatabase>(module, "CountryDatabase").
    def(init()).
    def(init<const CountryDatabase&>()).
    def_property_readonly("entries", &CountryDatabase::get_entries).
    def("from_code",
      overload_cast<CountryCode>(&CountryDatabase::from, const_)).
    def("from_name", &CountryDatabase::from_name).
    def("from_two_letter_code", overload_cast<
      CountryDatabase::TwoLetterCode>(&CountryDatabase::from, const_)).
    def("from_three_letter_code", overload_cast<
      CountryDatabase::ThreeLetterCode>(&CountryDatabase::from, const_)).
    def("add", &CountryDatabase::add).
    def("remove", &CountryDatabase::remove);
  class_<CountryDatabase::Entry>(country_database, "Entry").
    def(init()).
    def_readwrite("code", &CountryDatabase::Entry::m_code).
    def_readwrite("name", &CountryDatabase::Entry::m_name).
    def_readwrite(
      "two_letter_code", &CountryDatabase::Entry::m_two_letter_code).
    def_readwrite(
      "three_letter_code", &CountryDatabase::Entry::m_three_letter_code).
    def(self == self).
    def(self != self);
  ExportView<const CountryDatabase::Entry>(module, "CountryDatabaseEntryView");
  module.def("parse_country_code", static_cast<CountryCode (*)(
    std::string_view, const CountryDatabase&)>(parse_country_code));
  module.def("parse_country_code", static_cast<CountryCode (*)(
    std::string_view)>(parse_country_code));
  module.def("parse_country_database_entry", &parse_country_database_entry);
  module.def("parse_country_database", &parse_country_database);
}

void Nexus::Python::export_currency(module& module) {
  class_<CurrencyId>(module, "CurrencyId").
    def(init()).
    def(init<std::uint16_t>()).
    def_readonly_static("NONE", &CurrencyId::NONE).
    def("__int__", [] (CurrencyId self) {
      return static_cast<std::uint16_t>(self);
    }).
    def("__bool__", &CurrencyId::operator bool).
    def(self == self).
    def(self != self).
    def(self < self).
    def("__str__", &lexical_cast<std::string, CurrencyId>).
    def("__hash__", std::hash<CurrencyId>());
  auto currency_database = class_<CurrencyDatabase>(module, "CurrencyDatabase").
    def(init()).
    def(init<const CurrencyDatabase&>()).
    def_property_readonly("entries", &CurrencyDatabase::get_entries).
    def("from_id",
      overload_cast<CurrencyId>(&CurrencyDatabase::from, const_)).
    def("from_code",
      overload_cast<Beam::FixedString<3>>(&CurrencyDatabase::from, const_)).
    def("add", &CurrencyDatabase::add).
    def("remove", &CurrencyDatabase::remove);
  class_<CurrencyDatabase::Entry>(currency_database, "Entry").
    def(init()).
    def_readwrite("id", &CurrencyDatabase::Entry::m_id).
    def_readwrite("code", &CurrencyDatabase::Entry::m_code).
    def_readwrite("sign", &CurrencyDatabase::Entry::m_sign).
    def(self == self).
    def(self != self);
  ExportView<const CurrencyDatabase::Entry>(
    module, "CurrencyDatabaseEntryView");
  module.def("parse_currency",
    static_cast<CurrencyId (*)(std::string_view, const CurrencyDatabase&)>(
      parse_currency));
  module.def("parse_currency",
    static_cast<CurrencyId (*)(std::string_view)>(parse_currency));
  module.def("parse_currency_database_entry", &parse_currency_database_entry);
  module.def("parse_currency_database", &parse_currency_database);
}

void Nexus::Python::export_currency_pair(module& module) {
  class_<CurrencyPair>(module, "CurrencyPair").
    def(init()).
    def(init<const CurrencyPair&>()).
    def(init<CurrencyId, CurrencyId>()).
    def_readwrite("base", &CurrencyPair::m_base).
    def_readwrite("counter", &CurrencyPair::m_counter).
    def(self == self).
    def(self != self).
    def(self < self).
    def(self <= self).
    def(self > self).
    def(self >= self).
    def("__str__", &lexical_cast<std::string, CurrencyPair>).
    def("__hash__", std::hash<CurrencyPair>());
  module.def("parse_currency_pair", static_cast<CurrencyPair (*)(
    std::string_view, const CurrencyDatabase&)>(parse_currency_pair));
  module.def("parse_currency_pair",
    static_cast<CurrencyPair (*)(std::string_view)>(parse_currency_pair));
  module.def("invert", static_cast<CurrencyPair (*)(CurrencyPair)>(invert));
}

void Nexus::Python::export_default_countries(module& module) {
  module.attr("DEFAULT_COUNTRIES") =
    cast(DEFAULT_COUNTRIES, return_value_policy::reference);
  module.def("set_default_country_database",
    [] (const CountryDatabase& database) {
      set_default_countries(database);
    });
  auto submodule = module.def_submodule("default_countries");
  submodule.add_object("AU", cast(DefaultCountries::AU));
  submodule.add_object("BR", cast(DefaultCountries::BR));
  submodule.add_object("CA", cast(DefaultCountries::CA));
  submodule.add_object("CN", cast(DefaultCountries::CN));
  submodule.add_object("GB", cast(DefaultCountries::GB));
  submodule.add_object("HK", cast(DefaultCountries::HK));
  submodule.add_object("JP", cast(DefaultCountries::JP));
  submodule.add_object("US", cast(DefaultCountries::US));
}

void Nexus::Python::export_default_currencies(module& module) {
  module.attr("DEFAULT_CURRENCIES") =
    cast(DEFAULT_CURRENCIES, return_value_policy::reference);
  module.def("set_default_currencies", &set_default_currencies);
  auto submodule = module.def_submodule("default_currencies");
  submodule.add_object("AUD", cast(DefaultCurrencies::AUD));
  submodule.add_object("CAD", cast(DefaultCurrencies::CAD));
  submodule.add_object("EUR", cast(DefaultCurrencies::EUR));
  submodule.add_object("GBP", cast(DefaultCurrencies::GBP));
  submodule.add_object("HKD", cast(DefaultCurrencies::HKD));
  submodule.add_object("JPY", cast(DefaultCurrencies::JPY));
  submodule.add_object("USD", cast(DefaultCurrencies::USD));
  submodule.add_object("XBT", cast(DefaultCurrencies::XBT));
}

void Nexus::Python::export_default_destinations(module& module) {
  module.attr("DEFAULT_DESTINATIONS") =
    cast(DEFAULT_DESTINATIONS, return_value_policy::reference);
  module.def("set_default_destinations", &set_default_destinations);
  auto submodule = module.def_submodule("default_destinations");
  submodule.add_object("MOE", cast(DefaultDestinations::MOE));
  submodule.add_object("ASXT", cast(DefaultDestinations::ASXT));
  submodule.add_object("AMEX", cast(DefaultDestinations::AMEX));
  submodule.add_object("CBSX", cast(DefaultDestinations::CBSX));
  submodule.add_object("CXA", cast(DefaultDestinations::CXA));
  submodule.add_object("ARCA", cast(DefaultDestinations::ARCA));
  submodule.add_object("BATS", cast(DefaultDestinations::BATS));
  submodule.add_object("BATY", cast(DefaultDestinations::BATY));
  submodule.add_object("EDGA", cast(DefaultDestinations::EDGA));
  submodule.add_object("EDGX", cast(DefaultDestinations::EDGX));
  submodule.add_object("HKEX", cast(DefaultDestinations::HKEX));
  submodule.add_object("NYSE", cast(DefaultDestinations::NYSE));
  submodule.add_object("NASDAQ", cast(DefaultDestinations::NASDAQ));
  submodule.add_object("ALPHA", cast(DefaultDestinations::ALPHA));
  submodule.add_object("CHIX", cast(DefaultDestinations::CHIX));
  submodule.add_object("CSE", cast(DefaultDestinations::CSE));
  submodule.add_object("CSE2", cast(DefaultDestinations::CSE2));
  submodule.add_object("CX2", cast(DefaultDestinations::CX2));
  submodule.add_object("LYNX", cast(DefaultDestinations::LYNX));
  submodule.add_object("MATNLP", cast(DefaultDestinations::MATNLP));
  submodule.add_object("MATNMF", cast(DefaultDestinations::MATNMF));
  submodule.add_object("NEOE", cast(DefaultDestinations::NEOE));
  submodule.add_object("OMEGA", cast(DefaultDestinations::OMEGA));
  submodule.add_object("PURE", cast(DefaultDestinations::PURE));
  submodule.add_object("TSX", cast(DefaultDestinations::TSX));
  submodule.add_object("OSE", cast(DefaultDestinations::OSE));
  submodule.add_object("TSE", cast(DefaultDestinations::TSE));
}

void Nexus::Python::export_default_venues(module& module) {
  module.attr("DEFAULT_VENUES") =
    cast(DEFAULT_VENUES, return_value_policy::reference);
  module.def("set_default_venues", &set_default_venues);
  auto submodule = module.def_submodule("default_venues");
  submodule.add_object("HKEX", cast(DefaultVenues::HKEX));
  submodule.add_object("ASX", cast(DefaultVenues::ASX));
  submodule.add_object("CXA", cast(DefaultVenues::CXA));
  submodule.add_object("ASEX", cast(DefaultVenues::ASEX));
  submodule.add_object("ARCX", cast(DefaultVenues::ARCX));
  submodule.add_object("BATS", cast(DefaultVenues::BATS));
  submodule.add_object("BATY", cast(DefaultVenues::BATY));
  submodule.add_object("BOSX", cast(DefaultVenues::BOSX));
  submodule.add_object("CBOE", cast(DefaultVenues::CBOE));
  submodule.add_object("CSE", cast(DefaultVenues::CSE));
  submodule.add_object("CSE2", cast(DefaultVenues::CSE2));
  submodule.add_object("NSEX", cast(DefaultVenues::NSEX));
  submodule.add_object("ADFX", cast(DefaultVenues::ADFX));
  submodule.add_object("ISE", cast(DefaultVenues::ISE));
  submodule.add_object("EDGA", cast(DefaultVenues::EDGA));
  submodule.add_object("EDGX", cast(DefaultVenues::EDGX));
  submodule.add_object("PHLX", cast(DefaultVenues::PHLX));
  submodule.add_object("CHIC", cast(DefaultVenues::CHIC));
  submodule.add_object("LYNX", cast(DefaultVenues::LYNX));
  submodule.add_object("NASDAQ", cast(DefaultVenues::NASDAQ));
  submodule.add_object("NYSE", cast(DefaultVenues::NYSE));
  submodule.add_object("MATN", cast(DefaultVenues::MATN));
  submodule.add_object("NEOE", cast(DefaultVenues::NEOE));
  submodule.add_object("OMGA", cast(DefaultVenues::OMGA));
  submodule.add_object("PURE", cast(DefaultVenues::PURE));
  submodule.add_object("TSX", cast(DefaultVenues::TSX));
  submodule.add_object("TSXV", cast(DefaultVenues::TSXV));
  submodule.add_object("XATS", cast(DefaultVenues::XATS));
  submodule.add_object("XCX2", cast(DefaultVenues::XCX2));
  submodule.add_object("XFKA", cast(DefaultVenues::XFKA));
  submodule.add_object("TSE", cast(DefaultVenues::TSE));
  submodule.add_object("OSE", cast(DefaultVenues::OSE));
  submodule.add_object("NSE", cast(DefaultVenues::NSE));
  submodule.add_object("SSE", cast(DefaultVenues::SSE));
  submodule.add_object("CHIJ", cast(DefaultVenues::CHIJ));
}

void Nexus::Python::export_definitions(module& module) {
  export_bbo_quote(module);
  export_book_quote(module);
  export_country(module);
  export_currency(module);
  export_currency_pair(module);
  export_destination(module);
  export_venue(module);
  export_default_countries(module);
  export_default_currencies(module);
  export_default_destinations(module);
  export_default_venues(module);
  export_exchange_rate(module);
  export_exchange_rate_table(module);
  export_money(module);
  export_order_imbalance(module);
  export_order_status(module);
  export_order_type(module);
  export_quantity(module);
  export_quote(module);
  export_region(module);
  export_region_map(module);
  export_security(module);
  export_security_info(module);
  export_security_technicals(module);
  export_side(module);
  export_tag(module);
  export_time_and_sale(module);
  export_time_in_force(module);
  export_trading_schedule(module);
}

void Nexus::Python::export_destination(module& module) {
  auto destination_database =
    class_<DestinationDatabase>(module, "DestinationDatabase").
      def(init()).
      def(init<const DestinationDatabase&>()).
      def_property_readonly("entries", &DestinationDatabase::get_entries).
      def("from_id", &DestinationDatabase::from).
      def_property_readonly("preferred_destination",
        &DestinationDatabase::get_preferred_destination).
      def("select_first",
        [] (const DestinationDatabase& self, const object& predicate) {
          return self.select_first(
            [&] (const DestinationDatabase::Entry& entry) {
              return predicate(cast(entry)).cast<bool>();
            });
        }).
      def("select_all",
        [] (const DestinationDatabase& self, const object& predicate) {
          return self.select_all([&] (const DestinationDatabase::Entry& entry) {
            return predicate(cast(entry)).cast<bool>();
          });
        }).
      def_property("manual_order_entry_destination",
        &DestinationDatabase::get_manual_order_entry_destination,
        &DestinationDatabase::set_manual_order_entry_destination).
      def("add", &DestinationDatabase::add).
      def("set_preferred_destination",
        &DestinationDatabase::set_preferred_destination).
      def("remove", &DestinationDatabase::remove).
      def("remove_preferred_destination",
        &DestinationDatabase::remove_preferred_destination);
  class_<DestinationDatabase::Entry>(destination_database, "Entry").
    def(init()).
    def(init<const DestinationDatabase::Entry&>()).
    def_readwrite("id", &DestinationDatabase::Entry::m_id).
    def_readwrite("venues", &DestinationDatabase::Entry::m_venues).
    def_readwrite("description", &DestinationDatabase::Entry::m_description).
    def(self == self).
    def(self != self);
  ExportView<const DestinationDatabase::Entry>(
    module, "DestinationDatabaseEntryView");
  module.def("parse_destination_database_entry",
    static_cast<DestinationDatabase::Entry (*)(const YAML::Node&,
      const VenueDatabase&)>(&parse_destination_database_entry));
  module.def("parse_destination_database", static_cast<DestinationDatabase (*)(
    const YAML::Node&, const VenueDatabase&)>(&parse_destination_database));
}

void Nexus::Python::export_exchange_rate(module& module) {
  class_<ExchangeRate>(module, "ExchangeRate").
    def(init()).
    def(init<const ExchangeRate&>()).
    def(init<CurrencyPair, rational<int>>()).
    def_readwrite("pair", &ExchangeRate::m_pair).
    def_readwrite("rate", &ExchangeRate::m_rate).
    def(self == self).
    def(self != self).
    def("__str__", &lexical_cast<std::string, ExchangeRate>);
  module.def(
    "invert", static_cast<ExchangeRate (*)(const ExchangeRate&)>(&invert));
  module.def(
    "convert", static_cast<Money (*)(Money, const ExchangeRate&)>(&convert));
}

void Nexus::Python::export_exchange_rate_table(module& module) {
  class_<ExchangeRateTable>(module, "ExchangeRateTable").
    def(init()).
    def(init<const std::vector<ExchangeRate>&>()).
    def(init<const ExchangeRateTable&>()).
    def("find", &ExchangeRateTable::find).
    def("convert", static_cast<Money (ExchangeRateTable::*)(
      Money, CurrencyPair) const>(&ExchangeRateTable::convert)).
    def("convert", static_cast<Money (ExchangeRateTable::*)(
      Money, CurrencyId, CurrencyId) const>(&ExchangeRateTable::convert)).
    def("add", &ExchangeRateTable::add);
}

void Nexus::Python::export_money(module& module) {
  class_<Money>(module, "Money").
    def(init()).
    def(init<const Money&>()).
    def(init<double>()).
    def(init<Quantity>()).
    def_readonly_static("ZERO", &Money::ZERO).
    def_readonly_static("ONE", &Money::ONE).
    def_readonly_static("CENT", &Money::CENT).
    def_readonly_static("BIP", &Money::BIP).
    def_static("try_parse", &try_parse_money).
    def_static("parse", &parse_money).
    def("__str__", &lexical_cast<std::string, Money>).
    def("__abs__", static_cast<Money (*)(Money)>(&abs)).
    def("__floor__",
      bind_integer_precision(static_cast<Money (*)(Money, int)>(&floor))).
    def("__hash__", std::hash<Money>()).
    def("__ceil__",
      bind_integer_precision(static_cast<Money (*)(Money, int)>(&ceil))).
    def("__trunc__",
      bind_integer_precision(static_cast<Money (*)(Money, int)>(&truncate))).
    def("__round__",
      bind_integer_precision(static_cast<Money (*)(Money, int)>(&round))).
    def("__float__", [] (Money self) {
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
  implicitly_convertible<Quantity, Money>();
  module.def("abs", static_cast<Money (*)(Money)>(&abs));
  module.def("floor", static_cast<Money (*)(Money, int)>(&floor));
  module.def("ceil", static_cast<Money (*)(Money, int)>(&ceil));
  module.def("truncate", static_cast<Money (*)(Money, int)>(&truncate));
  module.def("round", static_cast<Money (*)(Money, int)>(&round));
  module.def("fmod", static_cast<Money (*)(Money, Money)>(&fmod));
}

void Nexus::Python::export_order_imbalance(module& module) {
  class_<OrderImbalance>(module, "OrderImbalance").
    def(init()).
    def(init<Security, Side, Quantity, Money, ptime>()).
    def(init<const OrderImbalance&>()).
    def_readwrite("security", &OrderImbalance::m_security).
    def_readwrite("side", &OrderImbalance::m_side).
    def_readwrite("size", &OrderImbalance::m_size).
    def_readwrite("reference_price", &OrderImbalance::m_reference_price).
    def_readwrite("timestamp", &OrderImbalance::m_timestamp).
    def("__str__", &lexical_cast<std::string, OrderImbalance>).
    def(self == self).
    def(self != self);
  ExportQueueSuite<OrderImbalance>(module, "OrderImbalance");
  ExportQueueSuite<SequencedOrderImbalance>(module, "SequencedOrderImbalance");
}

void Nexus::Python::export_order_status(module& module) {
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
  module.def("is_terminal", &is_terminal);
}

void Nexus::Python::export_order_type(module& module) {
  enum_<OrderType::Type>(module, "OrderType").
    value("NONE", OrderType::NONE).
    value("MARKET", OrderType::MARKET).
    value("LIMIT", OrderType::LIMIT).
    value("PEGGED", OrderType::PEGGED).
    value("STOP", OrderType::STOP);
}

void Nexus::Python::export_quantity(module& module) {
  class_<Quantity>(module, "Quantity").
    def(init()).
    def(init<int>()).
    def(init<double>()).
    def(init<const Quantity&>()).
    def_static("try_parse", &try_parse_quantity).
    def_static("parse", &parse_quantity).
    def("__str__", &lexical_cast<std::string, Quantity>).
    def("__abs__", static_cast<Quantity (*)(Quantity)>(&abs)).
    def("__floor__",
      bind_integer_precision(static_cast<Quantity (*)(Quantity, int)>(&floor))).
    def("__ceil__",
      bind_integer_precision(static_cast<Quantity (*)(Quantity, int)>(&ceil))).
    def("__hash__", std::hash<Quantity>()).
    def("__trunc__", bind_integer_precision(
      static_cast<Quantity (*)(Quantity, int)>(&truncate))).
    def("__round__",
      bind_integer_precision(static_cast<Quantity (*)(Quantity, int)>(&round))).
    def("__int__", [] (Quantity self) {
      return static_cast<int>(self);
    }).
    def("__float__", [] (Quantity self) {
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
  module.def("floor", static_cast<Quantity (*)(Quantity, int)>(&floor));
  module.def("ceil", static_cast<Quantity (*)(Quantity, int)>(&ceil));
  module.def("truncate", static_cast<Quantity (*)(Quantity, int)>(&truncate));
  module.def("round", static_cast<Quantity (*)(Quantity, int)>(&round));
}

void Nexus::Python::export_quote(module& module) {
  class_<Quote>(module, "Quote").
    def(init()).
    def(init<const Quote&>()).
    def(init<Money, Quantity, Side>()).
    def_readwrite("price", &Quote::m_price).
    def_readwrite("size", &Quote::m_size).
    def_readwrite("side", &Quote::m_side).
    def("__str__", &lexical_cast<std::string, Quote>).
    def(self == self).
    def(self != self);
  module.def("make_ask", &make_ask);
  module.def("make_bid", &make_bid);
  module.def("listing_comparator",
    static_cast<bool (*)(const Quote&, const Quote&)>(&listing_comparator));
  module.def("offer_comparator", &offer_comparator);
}

void Nexus::Python::export_region(module& module) {
  class_<Region>(module, "Region").
    def_readonly_static("GLOBAL", &Region::GLOBAL).
    def_static("make_global", &Region::make_global).
    def(init()).
    def(init<std::string>()).
    def(init<CountryCode>()).
    def(init<Venue>()).
    def(init<Security>()).
    def_property_readonly("name", &Region::get_name).
    def_property_readonly("is_global", &Region::is_global).
    def_property_readonly("is_empty", &Region::is_empty).
    def_property_readonly("countries", &Region::get_countries).
    def_property_readonly("venues", &Region::get_venues).
    def_property_readonly("securities", &Region::get_securities).
    def("contains", &Region::contains).
    def("__hash__", std::hash<Region>()).
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

void Nexus::Python::export_region_map(module& module) {
  using PythonRegionMap = RegionMap<object>;
  class_<PythonRegionMap>(module, "RegionMap").
    def(init<object>()).
    def(init<std::string, object>()).
    def_property_readonly("size", &PythonRegionMap::get_size).
    def("get", static_cast<
      const object& (PythonRegionMap::*)(const Region&) const>(
        &PythonRegionMap::get), return_value_policy::reference_internal).
    def("get", static_cast<object& (PythonRegionMap::*)(const Region&)>(
      &PythonRegionMap::get), return_value_policy::reference_internal).
    def("set", &PythonRegionMap::set).
    def("erase", &PythonRegionMap::erase).
    def("__getitem__", static_cast<const object& (PythonRegionMap::*)(
      const Region&) const>(&PythonRegionMap::get),
      return_value_policy::reference_internal).
    def("__setitem__", &PythonRegionMap::set).
    def("__delitem__", &PythonRegionMap::erase).
    def("__iter__", [] (const PythonRegionMap& p) {
      return make_iterator(p.begin(), p.end());
    }, keep_alive<0, 1>());
}

void Nexus::Python::export_security(module& module) {
  class_<Security>(module, "Security").
    def(init()).
    def(init<const Security&>()).
    def(init<std::string, Venue>()).
    def_property_readonly("symbol", &Security::get_symbol).
    def_property_readonly("venue", &Security::get_venue).
    def("__bool__", &Security::operator bool).
    def(self < self).
    def(self <= self).
    def(self == self).
    def(self != self).
    def(self >= self).
    def(self > self).
    def("__hash__", std::hash<Security>()).
    def("__str__", &lexical_cast<std::string, Security>);
  module.def("parse_security", static_cast<Security (*)(
    std::string_view, const VenueDatabase&)>(&parse_security));
  module.def("parse_security",
    static_cast<Security (*)(std::string_view)>(&parse_security));
  module.def("parse_security_set", static_cast<std::unordered_set<Security> (*)(
    const YAML::Node&, const VenueDatabase&)>(&parse_security_set));
  module.def("parse_security_set", static_cast<std::unordered_set<Security> (*)(
    const YAML::Node&)>(&parse_security_set));
}

void Nexus::Python::export_security_info(module& module) {
  class_<SecurityInfo>(module, "SecurityInfo").
    def(init()).
    def(init<const SecurityInfo&>()).
    def(init<Security, std::string, std::string, Quantity>()).
    def_readwrite("security", &SecurityInfo::m_security).
    def_readwrite("name", &SecurityInfo::m_name).
    def_readwrite("sector", &SecurityInfo::m_sector).
    def_readwrite("board_lot", &SecurityInfo::m_board_lot).
    def(self == self).
    def(self != self).
    def("__hash__", std::hash<SecurityInfo>()).
    def("__str__", &lexical_cast<std::string, SecurityInfo>);
}

void Nexus::Python::export_security_technicals(module& module) {
  class_<SecurityTechnicals>(module, "SecurityTechnicals").
    def(init()).
    def(init<const SecurityTechnicals&>()).
    def_readwrite("volume", &SecurityTechnicals::m_volume).
    def_readwrite("high", &SecurityTechnicals::m_high).
    def_readwrite("low", &SecurityTechnicals::m_low).
    def_readwrite("open", &SecurityTechnicals::m_open).
    def_readwrite("close", &SecurityTechnicals::m_close).
    def("__str__", &lexical_cast<std::string, SecurityTechnicals>);
}

void Nexus::Python::export_side(module& module) {
  enum_<Side::Type>(module, "Side").
    value("NONE", Side::NONE).
    value("ASK", Side::ASK).
    value("BID", Side::BID);
  module.def("pick", static_cast<
    const object& (*)(Side, const object&, const object&)>(&pick<object>));
  module.def("direction", &get_direction);
  module.def("side", &get_side);
  module.def("opposite", &get_opposite);
  module.def("to_char", static_cast<char (*)(Side)>(&to_char));
}

void Nexus::Python::export_tag(module& module) {
  class_<Tag>(module, "Tag").
    def(init()).
    def(init<const Tag&>()).
    def(init<int, Tag::Type>()).
    def_property_readonly("key", &Tag::get_key).
    def_property_readonly("value", &Tag::get_value).
    def("__str__", &lexical_cast<std::string, Tag>).
    def(self == self).
    def(self != self);
}

void Nexus::Python::export_time_and_sale(module& module) {
  auto outer = class_<TimeAndSale>(module, "TimeAndSale").
    def(init()).
    def(init<const TimeAndSale&>()).
    def(init<ptime, Money, Quantity, TimeAndSale::Condition, std::string,
      std::string, std::string>()).
    def_readwrite("timestamp", &TimeAndSale::m_timestamp).
    def_readwrite("price", &TimeAndSale::m_price).
    def_readwrite("size", &TimeAndSale::m_size).
    def_readwrite("condition", &TimeAndSale::m_condition).
    def_readwrite("market_center", &TimeAndSale::m_market_center).
    def_readwrite("buyer_mpid", &TimeAndSale::m_buyer_mpid).
    def_readwrite("seller_mpid", &TimeAndSale::m_seller_mpid).
    def("__str__", &lexical_cast<std::string, TimeAndSale>).
    def(self == self).
    def(self != self);
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
  ExportQueueSuite<TimeAndSale>(module, "TimeAndSale");
  ExportQueueSuite<SequencedTimeAndSale>(module, "SequencedTimeAndSale");
}

void Nexus::Python::export_time_in_force(module& module) {
  auto outer = class_<TimeInForce>(module, "TimeInForce").
    def(init()).
    def(init<const TimeInForce&>()).
    def(init<TimeInForce::Type>()).
    def(init<TimeInForce::Type, ptime>()).
    def_property_readonly("type", &TimeInForce::get_type).
    def_property_readonly("expiry", &TimeInForce::get_expiry).
    def("__hash__", std::hash<TimeInForce>()).
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
  module.def("make_gtd", &make_gtd);
}

void Nexus::Python::export_trading_schedule(module& module) {
  auto outer = class_<TradingSchedule>(module, "TradingSchedule").
    def(init<std::vector<TradingSchedule::Rule>>()).
    def("find", static_cast<std::vector<TradingSchedule::Event> (
      TradingSchedule::*)(date, Venue) const>(&TradingSchedule::find)).
    def("find", [] (
        const TradingSchedule& self, date date, Venue venue, const object& f) {
      return self.find(date, venue, [&] (const TradingSchedule::Event& event) {
        return f(cast(event)).cast<bool>();
      });
    });
  class_<TradingSchedule::Event>(outer, "Event").
    def(init()).
    def(init<const TradingSchedule::Event&>()).
    def_readwrite("code", &TradingSchedule::Event::m_code).
    def_readwrite("timestamp", &TradingSchedule::Event::m_timestamp).
    def("__str__", &lexical_cast<std::string, TradingSchedule::Event>).
    def(self == self).
    def(self != self);
  class_<TradingSchedule::Rule>(outer, "Rule").
    def(init()).
    def(init<const TradingSchedule::Rule&>()).
    def_readwrite("venues", &TradingSchedule::Rule::m_venues).
    def_readwrite("weekdays", &TradingSchedule::Rule::m_weekdays).
    def_readwrite("days", &TradingSchedule::Rule::m_days).
    def_readwrite("months", &TradingSchedule::Rule::m_months).
    def_readwrite("years", &TradingSchedule::Rule::m_years).
    def_readwrite("events", &TradingSchedule::Rule::m_events).
    def(self == self).
    def(self != self);
  module.def("is_match", &is_match);
  module.def("parse_trading_schedule", &parse_trading_schedule);
}

void Nexus::Python::export_venue(module& module) {
  class_<Venue>(module, "Venue").
    def(init()).
    def(init<Venue::Code>()).
    def_property_readonly("code", &Venue::get_code).
    def("__bool__", &Venue::operator bool).
    def(self < self).
    def(self <= self).
    def(self == self).
    def(self != self).
    def(self >= self).
    def(self > self).
    def("__hash__", std::hash<Venue>()).
    def("__str__", &lexical_cast<std::string, Venue>);
  auto venue_database = class_<VenueDatabase>(module, "VenueDatabase").
    def(init()).
    def(init<const VenueDatabase&>()).
    def_property_readonly("entries", &VenueDatabase::get_entries).
    def("from", overload_cast<Venue>(&VenueDatabase::from, const_)).
    def("from", overload_cast<std::string_view>(&VenueDatabase::from, const_)).
    def("from", overload_cast<CountryCode>(&VenueDatabase::from, const_)).
    def("from_display_name", &VenueDatabase::from_display_name).
    def("add", &VenueDatabase::add).
    def("remove", &VenueDatabase::remove);
  class_<VenueDatabase::Entry>(venue_database, "Entry").
    def(init()).
    def(init<const VenueDatabase::Entry&>()).
    def_readwrite("venue", &VenueDatabase::Entry::m_venue).
    def_readwrite("country_code", &VenueDatabase::Entry::m_country_code).
    def_readwrite("market_center", &VenueDatabase::Entry::m_market_center).
    def_readwrite("time_zone", &VenueDatabase::Entry::m_time_zone).
    def_readwrite("currency", &VenueDatabase::Entry::m_currency).
    def_readwrite("description", &VenueDatabase::Entry::m_description).
    def_readwrite("display_name", &VenueDatabase::Entry::m_display_name).
    def(self == self).
    def(self != self);
  ExportView<const VenueDatabase::Entry>(module, "VenueDatabaseEntryView");
  module.def("parse_venue", static_cast<
    Venue (*)(std::string_view, const VenueDatabase&)>(&parse_venue));
  module.def("parse_venue",
    static_cast<Venue (*)(std::string_view)>(&parse_venue));
  module.def("parse_venue_database_entry", &parse_venue_database_entry);
  module.def("parse_venue_database", &parse_venue_database);
}
