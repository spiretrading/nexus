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

void Nexus::Python::export_bbo_quote(module& module) {
  export_default_methods(class_<BboQuote>(module, "BboQuote")).
    def(init<Quote, Quote, ptime>()).
    def_readwrite("bid", &BboQuote::m_bid).
    def_readwrite("ask", &BboQuote::m_ask).
    def_readwrite("timestamp", &BboQuote::m_timestamp);
  export_queue_suite<BboQuote>(module, "BboQuote");
  export_queue_suite<SequencedBboQuote>(module, "SequencedBboQuote");
}

void Nexus::Python::export_book_quote(module& module) {
  export_default_methods(class_<BookQuote>(module, "BookQuote")).
    def(init<std::string, bool, Venue, Quote, ptime>()).
    def_readwrite("mpid", &BookQuote::m_mpid).
    def_readwrite("is_primary_mpid", &BookQuote::m_is_primary_mpid).
    def_readwrite("venue", &BookQuote::m_venue).
    def_readwrite("quote", &BookQuote::m_quote).
    def_readwrite("timestamp", &BookQuote::m_timestamp);
  export_queue_suite<BookQuote>(module, "BookQuote");
  export_queue_suite<SequencedBookQuote>(module, "SequencedBookQuote");
  module.def("is_same_listing", &is_same_listing);
  module.def("listing_comparator",
    overload_cast<const BookQuote&, const BookQuote&>(&listing_comparator));
}

void Nexus::Python::export_country(module& module) {
  export_default_methods(class_<CountryCode>(module, "CountryCode")).
    def(init<std::uint16_t>()).
    def_property_readonly_static("NONE", [] (const object&) {
      return CountryCode::NONE;
    });
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
  export_default_methods(
    class_<CountryDatabase::Entry>(country_database, "Entry")).
    def_readwrite("code", &CountryDatabase::Entry::m_code).
    def_readwrite("name", &CountryDatabase::Entry::m_name).
    def_readwrite(
      "two_letter_code", &CountryDatabase::Entry::m_two_letter_code).
    def_readwrite(
      "three_letter_code", &CountryDatabase::Entry::m_three_letter_code);
  export_view<const CountryDatabase::Entry>(module, "CountryDatabaseEntryView");
  module.def("parse_country_code",
    overload_cast<std::string_view, const CountryDatabase&>(parse_country_code));
  module.def("parse_country_code",
    overload_cast<std::string_view>(parse_country_code));
  module.def("parse_country_database_entry", &parse_country_database_entry);
  module.def("parse_country_database", &parse_country_database);
}

void Nexus::Python::export_currency(module& module) {
  export_default_methods(class_<CurrencyId>(module, "CurrencyId")).
    def(init<std::uint16_t>()).
    def_property_readonly_static("NONE", [] (const object&) {
      return CurrencyId::NONE;
    });
  auto currency_database =
    class_<CurrencyDatabase>(module, "CurrencyDatabase").
    def(init()).
    def(init<const CurrencyDatabase&>()).
    def_property_readonly("entries", &CurrencyDatabase::get_entries).
    def("from_id",
      overload_cast<CurrencyId>(&CurrencyDatabase::from, const_)).
    def("from_code",
      overload_cast<Beam::FixedString<3>>(&CurrencyDatabase::from, const_)).
    def("add", &CurrencyDatabase::add).
    def("remove", &CurrencyDatabase::remove);
  export_default_methods(
    class_<CurrencyDatabase::Entry>(currency_database, "Entry")).
    def_readwrite("id", &CurrencyDatabase::Entry::m_id).
    def_readwrite("code", &CurrencyDatabase::Entry::m_code).
    def_readwrite("sign", &CurrencyDatabase::Entry::m_sign);
  export_view<const CurrencyDatabase::Entry>(
    module, "CurrencyDatabaseEntryView");
  module.def("parse_currency",
    overload_cast<std::string_view, const CurrencyDatabase&>(parse_currency));
  module.def("parse_currency", overload_cast<std::string_view>(parse_currency));
  module.def("parse_currency_database_entry", &parse_currency_database_entry);
  module.def("parse_currency_database", &parse_currency_database);
}

void Nexus::Python::export_currency_pair(module& module) {
  export_default_methods(class_<CurrencyPair>(module, "CurrencyPair")).
    def(init<CurrencyId, CurrencyId>()).
    def_readwrite("base", &CurrencyPair::m_base).
    def_readwrite("counter", &CurrencyPair::m_counter);
  module.def("parse_currency_pair", overload_cast<
    std::string_view, const CurrencyDatabase&>(parse_currency_pair));
  module.def("parse_currency_pair",
    overload_cast<std::string_view>(parse_currency_pair));
  module.def("invert", overload_cast<CurrencyPair>(invert));
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
  submodule.add_object("CXA", cast(DefaultDestinations::CXA));
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
}

void Nexus::Python::export_default_venues(module& module) {
  module.attr("DEFAULT_VENUES") =
    cast(DEFAULT_VENUES, return_value_policy::reference);
  module.def("set_default_venues", &set_default_venues);
  auto submodule = module.def_submodule("default_venues");
  submodule.add_object("ASX", cast(DefaultVenues::ASX));
  submodule.add_object("CXA", cast(DefaultVenues::CXA));
  submodule.add_object("CSE", cast(DefaultVenues::CSE));
  submodule.add_object("CSE2", cast(DefaultVenues::CSE2));
  submodule.add_object("CHIC", cast(DefaultVenues::CHIC));
  submodule.add_object("CXD", cast(DefaultVenues::CXD));
  submodule.add_object("LYNX", cast(DefaultVenues::LYNX));
  submodule.add_object("MATN", cast(DefaultVenues::MATN));
  submodule.add_object("NEOE", cast(DefaultVenues::NEOE));
  submodule.add_object("OMGA", cast(DefaultVenues::OMGA));
  submodule.add_object("PURE", cast(DefaultVenues::PURE));
  submodule.add_object("TSX", cast(DefaultVenues::TSX));
  submodule.add_object("TSXV", cast(DefaultVenues::TSXV));
  submodule.add_object("XATS", cast(DefaultVenues::XATS));
  submodule.add_object("XCX2", cast(DefaultVenues::XCX2));
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
  export_default_methods(
    class_<DestinationDatabase::Entry>(destination_database, "Entry")).
    def_readwrite("id", &DestinationDatabase::Entry::m_id).
    def_readwrite("venues", &DestinationDatabase::Entry::m_venues).
    def_readwrite("description", &DestinationDatabase::Entry::m_description);
  export_view<const DestinationDatabase::Entry>(
    module, "DestinationDatabaseEntryView");
  module.def("parse_destination_database_entry", overload_cast<
    const YAML::Node&, const VenueDatabase&>(&parse_destination_database_entry));
  module.def("parse_destination_database", overload_cast<const YAML::Node&,
    const VenueDatabase&>(&parse_destination_database));
}

void Nexus::Python::export_exchange_rate(module& module) {
  export_default_methods(class_<ExchangeRate>(module, "ExchangeRate")).
    def(init<CurrencyPair, rational<int>>()).
    def_readwrite("pair", &ExchangeRate::m_pair).
    def_readwrite("rate", &ExchangeRate::m_rate);
  module.def("invert", overload_cast<const ExchangeRate&>(&invert));
  module.def("convert", overload_cast<Money, const ExchangeRate&>(&convert));
}

void Nexus::Python::export_exchange_rate_table(module& module) {
  export_default_methods(
    class_<ExchangeRateTable>(module, "ExchangeRateTable")).
    def(init<const std::vector<ExchangeRate>&>()).
    def("find", &ExchangeRateTable::find).
    def("convert",
      overload_cast<Money, CurrencyPair>(&ExchangeRateTable::convert, const_)).
    def("convert", overload_cast<Money, CurrencyId, CurrencyId>(
      &ExchangeRateTable::convert, const_)).
    def("add", &ExchangeRateTable::add);
}

void Nexus::Python::export_money(module& module) {
  auto money = export_default_methods(class_<Money>(module, "Money")).
    def(init<double>()).
    def(init<Quantity>()).
    def_property_readonly_static("ZERO", [] (const object&) {
      return Money::ZERO;
    }).
    def_property_readonly_static("ONE", [] (const object&) {
      return Money::ONE;
    }).
    def_property_readonly_static("CENT", [] (const object&) {
      return Money::CENT;
    }).
    def_property_readonly_static("BIP", [] (const object&) {
      return Money::BIP;
    }).
    def_static("try_parse", &try_parse_money).
    def_static("parse", &parse_money).
    def("__abs__", overload_cast<Money>(&abs)).
    def("__floor__", overload_cast<Money>(&floor)).
    def("__ceil__", overload_cast<Money>(&ceil)).
    def("__trunc__", overload_cast<Money>(&truncate)).
    def("__round__", overload_cast<Money>(&round)).
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
  module.def("abs", overload_cast<Money>(&abs));
  module.def("floor_to", overload_cast<Money, Money>(&floor_to));
  module.def("ceil_to", overload_cast<Money, Money>(&ceil_to));
  module.def("truncate_to", overload_cast<Money, Money>(&truncate_to));
  module.def("round_to", overload_cast<Money, Money>(&round_to));
  module.def("fmod", overload_cast<Money, Money>(&fmod));
}

void Nexus::Python::export_order_imbalance(module& module) {
  export_default_methods(class_<OrderImbalance>(module, "OrderImbalance")).
    def(init<Security, Side, Quantity, Money, ptime>()).
    def_readwrite("security", &OrderImbalance::m_security).
    def_readwrite("side", &OrderImbalance::m_side).
    def_readwrite("size", &OrderImbalance::m_size).
    def_readwrite("reference_price", &OrderImbalance::m_reference_price).
    def_readwrite("timestamp", &OrderImbalance::m_timestamp);
  export_queue_suite<OrderImbalance>(module, "OrderImbalance");
  export_queue_suite<SequencedOrderImbalance>(module, "SequencedOrderImbalance");
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
  auto quantity = export_default_methods(class_<Quantity>(module, "Quantity")).
    def(init<int>()).
    def(init<double>()).
    def_static("try_parse", &try_parse_quantity).
    def_static("parse", &parse_quantity).
    def("__abs__", overload_cast<Quantity>(&abs)).
    def("__floor__", overload_cast<Quantity>(&floor)).
    def("__ceil__", overload_cast<Quantity>(&ceil)).
    def("__trunc__", overload_cast<Quantity>(&truncate)).
    def("__round__", overload_cast<Quantity>(&round)).
    def(self < double()).
    def(double() < self).
    def(self < int()).
    def(int() < self).
    def(self <= double()).
    def(double() <= self).
    def(self <= int()).
    def(int() <= self).
    def(self == double()).
    def(double() == self).
    def(self == int()).
    def(int() == self).
    def(self != double()).
    def(double() != self).
    def(self != int()).
    def(int() != self).
    def(self >= double()).
    def(double() >= self).
    def(self >= int()).
    def(int() >= self).
    def(self > double()).
    def(double() > self).
    def(self > int()).
    def(int() > self).
    def(self + double()).
    def(double() + self).
    def(self + int()).
    def(int() + self).
    def(self - double()).
    def(double() - self).
    def(self - int()).
    def(int() - self).
    def(self * double()).
    def(double() * self).
    def(self * int()).
    def(int() * self).
    def(self / double()).
    def(double() / self).
    def(self / int()).
    def(int() / self);
  implicitly_convertible<std::int32_t, Quantity>();
  implicitly_convertible<std::int64_t, Quantity>();
  implicitly_convertible<double, Quantity>();
  module.def("abs", overload_cast<Quantity>(&abs));
  module.def("floor_to", overload_cast<Quantity, Quantity>(&floor_to));
  module.def("ceil_to", overload_cast<Quantity, Quantity>(&ceil_to));
  module.def("truncate_to", overload_cast<Quantity, Quantity>(&truncate_to));
  module.def("round_to", overload_cast<Quantity, Quantity>(&round_to));
  module.def("fmod", overload_cast<Quantity, Quantity>(&fmod));
}

void Nexus::Python::export_quote(module& module) {
  export_default_methods(class_<Quote>(module, "Quote")).
    def(init<Money, Quantity, Side>()).
    def_readwrite("price", &Quote::m_price).
    def_readwrite("size", &Quote::m_size).
    def_readwrite("side", &Quote::m_side);
  module.def("make_ask", &make_ask);
  module.def("make_bid", &make_bid);
  module.def("listing_comparator",
    overload_cast<const Quote&, const Quote&>(&listing_comparator));
  module.def("offer_comparator", &offer_comparator);
}

void Nexus::Python::export_region(module& module) {
  export_default_methods(class_<Region>(module, "Region")).
    def_property_readonly_static("GLOBAL", [] (const object&) { return Region::GLOBAL; }).
    def_static("make_global", &Region::make_global).
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
    def("contains", &Region::contains);
  implicitly_convertible<CountryCode, Region>();
  implicitly_convertible<Venue, Region>();
  implicitly_convertible<Security, Region>();
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
  export_default_methods(class_<Security>(module, "Security")).
    def(init<std::string, Venue>()).
    def_property_readonly("symbol", &Security::get_symbol).
    def_property_readonly("venue", &Security::get_venue);
  module.def("parse_security",
    overload_cast<std::string_view, const VenueDatabase&>(&parse_security));
  module.def("parse_security", overload_cast<std::string_view>(&parse_security));
  module.def("parse_security_set", overload_cast<
    const YAML::Node&, const VenueDatabase&>(&parse_security_set));
  module.def("parse_security_set",
    overload_cast<const YAML::Node&>(&parse_security_set));
}

void Nexus::Python::export_security_info(module& module) {
  export_default_methods(class_<SecurityInfo>(module, "SecurityInfo")).
    def(init<Security, std::string, std::string, Quantity>()).
    def_readwrite("security", &SecurityInfo::m_security).
    def_readwrite("name", &SecurityInfo::m_name).
    def_readwrite("sector", &SecurityInfo::m_sector).
    def_readwrite("board_lot", &SecurityInfo::m_board_lot);
}

void Nexus::Python::export_security_technicals(module& module) {
  export_default_methods(
    class_<SecurityTechnicals>(module, "SecurityTechnicals")).
    def_readwrite("volume", &SecurityTechnicals::m_volume).
    def_readwrite("high", &SecurityTechnicals::m_high).
    def_readwrite("low", &SecurityTechnicals::m_low).
    def_readwrite("open", &SecurityTechnicals::m_open).
    def_readwrite("close", &SecurityTechnicals::m_close);
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
  export_default_methods(class_<Tag>(module, "Tag")).
    def(init<int, Tag::Type>()).
    def_property_readonly("key", &Tag::get_key).
    def_property_readonly("value", &Tag::get_value);
}

void Nexus::Python::export_time_and_sale(module& module) {
  auto outer = export_default_methods(
    class_<TimeAndSale>(module, "TimeAndSale")).
    def(init<ptime, Money, Quantity, TimeAndSale::Condition, std::string,
      std::string, std::string>()).
    def_readwrite("timestamp", &TimeAndSale::m_timestamp).
    def_readwrite("price", &TimeAndSale::m_price).
    def_readwrite("size", &TimeAndSale::m_size).
    def_readwrite("condition", &TimeAndSale::m_condition).
    def_readwrite("market_center", &TimeAndSale::m_market_center).
    def_readwrite("buyer_mpid", &TimeAndSale::m_buyer_mpid).
    def_readwrite("seller_mpid", &TimeAndSale::m_seller_mpid);
  export_default_methods(class_<TimeAndSale::Condition>(outer, "Condition")).
    def_readwrite("type", &TimeAndSale::Condition::m_type).
    def_readwrite("code", &TimeAndSale::Condition::m_code);
  enum_<TimeAndSale::Condition::Type::Type>(outer, "Type").
    value("NONE", TimeAndSale::Condition::Type::NONE).
    value("REGULAR", TimeAndSale::Condition::Type::REGULAR).
    value("OPEN", TimeAndSale::Condition::Type::OPEN).
    value("CLOSE", TimeAndSale::Condition::Type::CLOSE);
  export_queue_suite<TimeAndSale>(module, "TimeAndSale");
  export_queue_suite<SequencedTimeAndSale>(module, "SequencedTimeAndSale");
}

void Nexus::Python::export_time_in_force(module& module) {
  auto outer = export_default_methods(
    class_<TimeInForce>(module, "TimeInForce")).
    def(init<TimeInForce::Type>()).
    def(init<TimeInForce::Type, ptime>()).
    def_property_readonly("type", &TimeInForce::get_type).
    def_property_readonly("expiry", &TimeInForce::get_expiry);
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
    def("find", [] (const TradingSchedule& self, date date, Venue venue) {
      return self.find(date, venue);
    }).
    def("find", [] (
        const TradingSchedule& self, date date, Venue venue, const object& f) {
      return self.find(date, venue, [&] (const TradingSchedule::Event& event) {
        return f(cast(event)).cast<bool>();
      });
    });
  export_default_methods(class_<TradingSchedule::Event>(outer, "Event")).
    def_readwrite("code", &TradingSchedule::Event::m_code).
    def_readwrite("timestamp", &TradingSchedule::Event::m_timestamp);
  export_default_methods(class_<TradingSchedule::Rule>(outer, "Rule")).
    def_readwrite("venues", &TradingSchedule::Rule::m_venues).
    def_readwrite("weekdays", &TradingSchedule::Rule::m_weekdays).
    def_readwrite("days", &TradingSchedule::Rule::m_days).
    def_readwrite("months", &TradingSchedule::Rule::m_months).
    def_readwrite("years", &TradingSchedule::Rule::m_years).
    def_readwrite("events", &TradingSchedule::Rule::m_events);
  module.def("is_match", &is_match);
  module.def("parse_trading_schedule", &parse_trading_schedule);
}

void Nexus::Python::export_venue(module& module) {
  export_default_methods(class_<Venue>(module, "Venue")).
    def(init<Venue::Code>()).
    def_property_readonly("code", &Venue::get_code);
  auto venue_database = class_<VenueDatabase>(module, "VenueDatabase").
    def(init()).
    def(init<const VenueDatabase&>()).
    def_property_readonly("entries", &VenueDatabase::get_entries).
    def("select", overload_cast<Venue>(&VenueDatabase::from, const_)).
    def(
      "select", overload_cast<std::string_view>(&VenueDatabase::from, const_)).
    def("select", overload_cast<CountryCode>(&VenueDatabase::from, const_)).
    def("select_display_name", &VenueDatabase::from_display_name).
    def("add", &VenueDatabase::add).
    def("remove", &VenueDatabase::remove);
  export_default_methods(
    class_<VenueDatabase::Entry>(venue_database, "Entry")).
    def_readwrite("venue", &VenueDatabase::Entry::m_venue).
    def_readwrite("country_code", &VenueDatabase::Entry::m_country_code).
    def_readwrite("market_center", &VenueDatabase::Entry::m_market_center).
    def_readwrite("time_zone", &VenueDatabase::Entry::m_time_zone).
    def_readwrite("currency", &VenueDatabase::Entry::m_currency).
    def_readwrite("description", &VenueDatabase::Entry::m_description).
    def_readwrite("display_name", &VenueDatabase::Entry::m_display_name);
  export_view<const VenueDatabase::Entry>(module, "VenueDatabaseEntryView");
  module.def("parse_venue_entry",
    overload_cast<std::string_view, const VenueDatabase&>(&parse_venue_entry));
  module.def(
    "parse_venue_entry", overload_cast<std::string_view>(&parse_venue_entry));
  module.def("parse_venue",
    overload_cast<std::string_view, const VenueDatabase&>(&parse_venue));
  module.def("parse_venue", overload_cast<std::string_view>(&parse_venue));
  module.def("parse_venue_database_entry", &parse_venue_database_entry);
  module.def("parse_venue_database", &parse_venue_database);
  module.def("utc_to_venue", &utc_to_venue);
  module.def("venue_to_utc", &venue_to_utc);
  module.def("utc_start_of_day", &utc_start_of_day);
  module.def("utc_end_of_day", &utc_end_of_day);
}
