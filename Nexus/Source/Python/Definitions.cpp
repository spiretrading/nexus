#include "Nexus/Python/Definitions.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Enum.hpp>
#include <Beam/Python/FixedString.hpp>
#include <Beam/Python/Optional.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/Variant.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/CurrencyPair.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::Python;
using namespace std;

void Nexus::Python::ExportBboQuote() {
  class_<BboQuote>("BboQuote", init<>())
    .def(init<const Quote&, const Quote&, const ptime&>())
    .def_readwrite("bid", &BboQuote::m_bid)
    .def_readwrite("ask", &BboQuote::m_ask)
    .add_property("timestamp", make_getter(&BboQuote::m_timestamp,
      return_value_policy<return_by_value>()), make_setter(
      &BboQuote::m_timestamp, return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportBookQuote() {
  class_<BookQuote>("BookQuote", init<>())
    .def(init<string, bool, MarketCode, const Quote&, const ptime&>())
    .def_readwrite("mpid", &BookQuote::m_mpid)
    .def_readwrite("is_primary_mpid", &BookQuote::m_isPrimaryMpid)
    .def_readwrite("market", &BookQuote::m_market)
    .add_property("timestamp", make_getter(&BookQuote::m_timestamp,
      return_value_policy<return_by_value>()), make_setter(
      &BookQuote::m_timestamp, return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportCountry() {
  {
    scope outer =
      class_<CountryDatabase>("CountryDatabase", init<>())
        .add_property("entries", make_function(&CountryDatabase::GetEntries,
          return_internal_reference<>()))
        .def("from_code", &CountryDatabase::FromCode,
          return_value_policy<copy_const_reference>())
        .def("from_name", &CountryDatabase::FromName,
          return_value_policy<copy_const_reference>())
        .def("from_two_letter_code", &CountryDatabase::FromTwoLetterCode,
          return_value_policy<copy_const_reference>())
        .def("from_three_letter_code", &CountryDatabase::FromThreeLetterCode,
          return_value_policy<copy_const_reference>())
        .def("add", &CountryDatabase::Add)
        .def("delete", &CountryDatabase::Delete);
      class_<CountryDatabase::Entry>("Entry")
        .def_readwrite("code", &CountryDatabase::Entry::m_code)
        .def_readwrite("name", &CountryDatabase::Entry::m_name)
        .def_readwrite("two_letter_code",
          &CountryDatabase::Entry::m_twoLetterCode)
        .def_readwrite("three_letter_code",
          &CountryDatabase::Entry::m_threeLetterCode);
  }
  class_<vector<CountryDatabase::Entry>>("VectorCountryDatabaseEntry")
    .def(vector_indexing_suite<vector<CountryDatabase::Entry>>());
  ExportFixedString<2>();
  ExportFixedString<3>();
}

void Nexus::Python::ExportCurrency() {
  class_<CurrencyId>("CurrencyId", init<>())
    .def(init<int>())
    .add_static_property("NONE", &CurrencyId::NONE)
    .def_readwrite("value", &CurrencyId::m_value)
    .def(self == self)
    .def(self != self)
    .def(self < self);
  {
    scope outer =
      class_<CurrencyDatabase>("CurrencyDatabase", init<>())
        .add_property("entries", make_function(&CurrencyDatabase::GetEntries,
          return_internal_reference<>()))
        .def("from_id", &CurrencyDatabase::FromId,
          return_value_policy<copy_const_reference>())
        .def("from_code", &CurrencyDatabase::FromCode,
          return_value_policy<copy_const_reference>())
        .def("add", &CurrencyDatabase::Add)
        .def("delete", &CurrencyDatabase::Delete);
      class_<CurrencyDatabase::Entry>("Entry")
        .def_readwrite("id", &CurrencyDatabase::Entry::m_id)
        .add_property("code", make_getter(&CurrencyDatabase::Entry::m_code,
          return_value_policy<return_by_value>()),
          make_setter(&CurrencyDatabase::Entry::m_code,
          return_value_policy<return_by_value>()))
        .def_readwrite("sign", &CurrencyDatabase::Entry::m_sign);
  }
  class_<vector<CurrencyDatabase::Entry>>("VectorCurrencyDatabaseEntry")
    .def(vector_indexing_suite<vector<CurrencyDatabase::Entry>>());
}

void Nexus::Python::ExportCurrencyPair() {
  class_<CurrencyPair>("CurrencyPair", init<>())
    .def(init<CurrencyId, CurrencyId>())
    .def_readwrite("base", &CurrencyPair::m_base)
    .def_readwrite("counter", &CurrencyPair::m_counter);
  def("parse_currency_pair",
    static_cast<CurrencyPair (*)(const string&, const CurrencyDatabase&)>(
    &ParseCurrencyPair));
  def("parse_currency_pair",
    static_cast<CurrencyPair (*)(const string&)>(&ParseCurrencyPair));
  def("invert", static_cast<CurrencyPair (*)(const CurrencyPair&)>(&Invert));
}

void Nexus::Python::ExportDefaultCurrencies() {
  struct Dummy {};
  class_<Dummy, noncopyable>("default_currencies", no_init)
    .add_static_property("AUD", make_function(&DefaultCurrencies::AUD))
    .add_static_property("CAD", make_function(&DefaultCurrencies::CAD))
    .add_static_property("USD", make_function(&DefaultCurrencies::USD));
}

void Nexus::Python::ExportDefaultDestinations() {
  struct Dummy {};
  class_<Dummy, noncopyable>("default_destinations", no_init)
    .add_static_property("MOE", make_function(&DefaultDestinations::MOE,
      return_value_policy<copy_const_reference>()))
    .add_static_property("ASXT", make_function(&DefaultDestinations::ASXT,
      return_value_policy<copy_const_reference>()))
    .add_static_property("CXA", make_function(&DefaultDestinations::CXA,
      return_value_policy<copy_const_reference>()))
    .add_static_property("ARCA", make_function(&DefaultDestinations::ARCA,
      return_value_policy<copy_const_reference>()))
    .add_static_property("BATS", make_function(&DefaultDestinations::BATS,
      return_value_policy<copy_const_reference>()))
    .add_static_property("NYSE", make_function(&DefaultDestinations::NYSE,
      return_value_policy<copy_const_reference>()))
    .add_static_property("NASDAQ", make_function(&DefaultDestinations::NASDAQ,
      return_value_policy<copy_const_reference>()))
    .add_static_property("ALPHA", make_function(&DefaultDestinations::ALPHA,
      return_value_policy<copy_const_reference>()))
    .add_static_property("CHIX", make_function(&DefaultDestinations::CHIX,
      return_value_policy<copy_const_reference>()))
    .add_static_property("CX2", make_function(&DefaultDestinations::CX2,
      return_value_policy<copy_const_reference>()))
    .add_static_property("LYNX", make_function(&DefaultDestinations::LYNX,
      return_value_policy<copy_const_reference>()))
    .add_static_property("MATNLP", make_function(&DefaultDestinations::MATNLP,
      return_value_policy<copy_const_reference>()))
    .add_static_property("MATNMF", make_function(&DefaultDestinations::MATNMF,
      return_value_policy<copy_const_reference>()))
    .add_static_property("NEOE", make_function(&DefaultDestinations::NEOE,
      return_value_policy<copy_const_reference>()))
    .add_static_property("OMEGA", make_function(&DefaultDestinations::OMEGA,
      return_value_policy<copy_const_reference>()))
    .add_static_property("PURE", make_function(&DefaultDestinations::PURE,
      return_value_policy<copy_const_reference>()))
    .add_static_property("TSX", make_function(&DefaultDestinations::TSX,
      return_value_policy<copy_const_reference>()));
}

void Nexus::Python::ExportDefaultMarkets() {
  struct Dummy {};
  class_<Dummy, noncopyable>("default_markets", no_init)
    .add_static_property("ASX", make_function(&DefaultMarkets::ASX))
    .add_static_property("CXA", make_function(&DefaultMarkets::CXA))
    .add_static_property("ASEX", make_function(&DefaultMarkets::ASEX))
    .add_static_property("ARCX", make_function(&DefaultMarkets::ARCX))
    .add_static_property("BATS", make_function(&DefaultMarkets::BATS))
    .add_static_property("BATY", make_function(&DefaultMarkets::BATY))
    .add_static_property("BOSX", make_function(&DefaultMarkets::BOSX))
    .add_static_property("CBOE", make_function(&DefaultMarkets::CBOE))
    .add_static_property("NSEX", make_function(&DefaultMarkets::NSEX))
    .add_static_property("ADFX", make_function(&DefaultMarkets::ADFX))
    .add_static_property("ISE", make_function(&DefaultMarkets::ISE))
    .add_static_property("EDGA", make_function(&DefaultMarkets::EDGA))
    .add_static_property("EDGX", make_function(&DefaultMarkets::EDGX))
    .add_static_property("PHLX", make_function(&DefaultMarkets::PHLX))
    .add_static_property("CHIC", make_function(&DefaultMarkets::CHIC))
    .add_static_property("LYNX", make_function(&DefaultMarkets::LYNX))
    .add_static_property("NASDAQ", make_function(&DefaultMarkets::NASDAQ))
    .add_static_property("NYSE", make_function(&DefaultMarkets::NYSE))
    .add_static_property("MATN", make_function(&DefaultMarkets::MATN))
    .add_static_property("NEOE", make_function(&DefaultMarkets::NEOE))
    .add_static_property("OMGA", make_function(&DefaultMarkets::OMGA))
    .add_static_property("PURE", make_function(&DefaultMarkets::PURE))
    .add_static_property("TSX", make_function(&DefaultMarkets::TSX))
    .add_static_property("TSXV", make_function(&DefaultMarkets::TSXV))
    .add_static_property("XATS", make_function(&DefaultMarkets::XATS))
    .add_static_property("XCX2", make_function(&DefaultMarkets::XCX2));
}

void Nexus::Python::ExportDestination() {
  {
    scope outer =
      class_<DestinationDatabase>("DestinationDatabase", init<>())
        .def("from_id", &DestinationDatabase::FromId,
          return_value_policy<copy_const_reference>())
        .def("get_preferred_destination",
          &DestinationDatabase::GetPreferredDestination,
          return_value_policy<copy_const_reference>())
        .def("select_entry",
          static_cast<const DestinationDatabase::Entry&
          (DestinationDatabase::*)(object) const>(
          &DestinationDatabase::SelectEntry<object>),
          return_value_policy<copy_const_reference>())
        .def("select_entries",
          static_cast<vector<DestinationDatabase::Entry>
          (DestinationDatabase::*)(object) const>(
          &DestinationDatabase::SelectEntries<object>))
        .add_property("manual_order_entry_destination",
          make_function(&DestinationDatabase::GetManualOrderEntryDestination,
          return_value_policy<copy_const_reference>()),
          &DestinationDatabase::SetManualOrderEntryDestination)
        .def("add", &DestinationDatabase::Add)
        .def("set_preferred_destination",
          &DestinationDatabase::SetPreferredDesintation)
        .def("delete", &DestinationDatabase::Delete)
        .def("delete_preferred_destination",
          &DestinationDatabase::DeletePreferredDestination);
      class_<DestinationDatabase::Entry>("Entry")
        .def_readwrite("id", &DestinationDatabase::Entry::m_id)
        .def_readwrite("markets", &DestinationDatabase::Entry::m_markets)
        .def_readwrite("description",
          &DestinationDatabase::Entry::m_description);
  }
  class_<vector<DestinationDatabase::Entry>>("VectorDestinationDatabaseEntry")
    .def(vector_indexing_suite<vector<DestinationDatabase::Entry>>());
}

void Nexus::Python::ExportDefinitions() {
  ExportSecurity();
  ExportBboQuote();
  ExportBookQuote();
  ExportCountry();
  ExportCurrency();
  ExportCurrencyPair();
  ExportDestination();
  ExportExchangeRate();
  ExportExchangeRateTable();
  ExportMarket();
  ExportMarketQuote();
  ExportMoney();
  ExportOrderImbalance();
  ExportOrderStatus();
  ExportOrderType();
  ExportQuote();
  ExportSide();
  ExportTag();
  ExportTimeAndSale();
  ExportTimeInForce();
  ExportDefaultCurrencies();
  ExportDefaultDestinations();
  ExportDefaultMarkets();
}

void Nexus::Python::ExportExchangeRate() {
  class_<ExchangeRate>("ExchangeRate", init<>())
    .def(init<const CurrencyPair&, const rational<int>&>())
    .def_readwrite("pair", &ExchangeRate::m_pair)
    .def_readwrite("rate", &ExchangeRate::m_rate);
  def("invert", static_cast<ExchangeRate (*)(const ExchangeRate&)>(&Invert));
  def("convert", &Convert);
}

void Nexus::Python::ExportExchangeRateTable() {
  class_<ExchangeRateTable, noncopyable>("ExchangeRateTable", init<>())
    .def("find", &ExchangeRateTable::Find)
    .def("convert", &ExchangeRateTable::Convert)
    .def("add", &ExchangeRateTable::Add);
}

void Nexus::Python::ExportMarket() {
  {
    scope outer =
      class_<MarketDatabase>("MarketDatabase", init<>())
        .add_property("entries", make_function(
          &MarketDatabase::GetEntries, return_internal_reference<>()))
        .def("from_code", &MarketDatabase::FromCode,
          return_value_policy<copy_const_reference>())
        .def("from_display_name", &MarketDatabase::FromDisplayName,
          return_value_policy<copy_const_reference>())
        .def("from_country", &MarketDatabase::FromCountry)
        .def("add", &MarketDatabase::Add)
        .def("delete", &MarketDatabase::Delete);
      class_<MarketDatabase::Entry>("Entry")
        .add_property("code", make_getter(&MarketDatabase::Entry::m_code,
          return_value_policy<return_by_value>()),
          make_setter(&MarketDatabase::Entry::m_code,
          return_value_policy<return_by_value>()))
        .def_readwrite("country_code", &MarketDatabase::Entry::m_countryCode)
        .def_readwrite("time_zone", &MarketDatabase::Entry::m_timeZone)
        .def_readwrite("currency", &MarketDatabase::Entry::m_currency)
        .def_readwrite("board_lot", &MarketDatabase::Entry::m_boardLot)
        .def_readwrite("description", &MarketDatabase::Entry::m_description)
        .def_readwrite("display_name", &MarketDatabase::Entry::m_displayName);
  }
  class_<vector<MarketDatabase::Entry>>("VectorMarketDatabaseEntry")
    .def(vector_indexing_suite<vector<MarketDatabase::Entry>>());
}

void Nexus::Python::ExportMarketQuote() {
  class_<MarketQuote>("MarketQuote", init<>())
    .def(init<MarketCode, const Quote&, const Quote&, const ptime&>())
    .add_property("market", make_getter(&MarketQuote::m_market,
      return_value_policy<return_by_value>()), make_setter(
      &MarketQuote::m_market, return_value_policy<return_by_value>()))
    .def_readwrite("bid", &MarketQuote::m_bid)
    .def_readwrite("ask", &MarketQuote::m_ask)
    .add_property("timestamp", make_getter(&MarketQuote::m_timestamp,
      return_value_policy<return_by_value>()), make_setter(
      &MarketQuote::m_timestamp, return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportMoney() {
  class_<Money>("Money", init<>())
    .def_readonly("ZERO", Money::ZERO)
    .def_readonly("ONE", Money::ONE)
    .def_readonly("CENT", Money::CENT)
    .def_readonly("BIP", Money::BIP)
    .def_readonly("EPSILON", Money::EPSILON)
    .def("from_value", static_cast<Money (*)(double)>(&Money::FromValue))
    .def("from_value",
      static_cast<boost::optional<Money> (*)(const std::string&)>(
      &Money::FromValue))
    .staticmethod("from_value")
    .def("__str__", &Money::ToString)
    .def("__abs__", &Abs)
    .def("__float__", &ToDouble)
    .def(self < self)
    .def(self <= self)
    .def(self == self)
    .def(self != self)
    .def(self >= self)
    .def(self > self)
    .def(self + self)
    .def(self - self)
    .def(self / self)
    .def(-self)
    .def(int() * self)
    .def(double() * self)
    .def(self / int())
    .def(self / double());
  def("floor", &Floor);
  def("ceil", &Ceil);
  def("truncate", &Truncate);
  def("round", &Round);
  python_optional<Money>();
}

void Nexus::Python::ExportOrderImbalance() {
  class_<OrderImbalance>("OrderImbalance", init<>())
    .def(init<Security, Side, Quantity, Money, const ptime&>())
    .def_readwrite("security", &OrderImbalance::m_security)
    .add_property("side", make_getter(&OrderImbalance::m_side,
      return_value_policy<return_by_value>()), make_setter(
      &OrderImbalance::m_side, return_value_policy<return_by_value>()))
    .def_readwrite("size", &OrderImbalance::m_size)
    .def_readwrite("reference_price", &OrderImbalance::m_referencePrice)
    .add_property("timestamp", make_getter(&OrderImbalance::m_timestamp,
      return_value_policy<return_by_value>()), make_setter(
      &OrderImbalance::m_timestamp, return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportOrderStatus() {
  enum_<OrderStatus::Type>("OrderStatus")
    .value("NONE", OrderStatus::NONE)
    .value("PENDING_NEW", OrderStatus::PENDING_NEW)
    .value("REJECTED", OrderStatus::REJECTED)
    .value("NEW", OrderStatus::NEW)
    .value("PARTIALLY_FILLED", OrderStatus::PARTIALLY_FILLED)
    .value("EXPIRED", OrderStatus::EXPIRED)
    .value("CANCELED", OrderStatus::CANCELED)
    .value("SUSPENDED", OrderStatus::SUSPENDED)
    .value("STOPPED", OrderStatus::STOPPED)
    .value("FILLED", OrderStatus::FILLED)
    .value("DONE_FOR_DAY", OrderStatus::DONE_FOR_DAY)
    .value("PENDING_CANCEL", OrderStatus::PENDING_CANCEL)
    .value("CANCEL_REJECT", OrderStatus::CANCEL_REJECT);
  ExportEnum<OrderStatus>();
  def("is_terminal", &IsTerminal);
}

void Nexus::Python::ExportOrderType() {
  enum_<OrderType::Type>("OrderType")
    .value("NONE", OrderType::NONE)
    .value("MARKET", OrderType::MARKET)
    .value("LIMIT", OrderType::LIMIT)
    .value("PEGGED", OrderType::PEGGED)
    .value("STOP", OrderType::STOP);
  ExportEnum<OrderType>();
}

void Nexus::Python::ExportQuote() {
  class_<Quote>("Quote", init<>())
    .def(init<Money, Quantity, Side>())
    .def_readwrite("price", &Quote::m_price)
    .def_readwrite("size", &Quote::m_size)
    .add_property("side", make_getter(&Quote::m_side,
      return_value_policy<return_by_value>()), make_setter(&Quote::m_side,
      return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportSecurity() {
  class_<Security>("Security", init<>())
    .def(init<const string&, MarketCode, CountryCode>())
    .def(init<const string&, CountryCode>())
    .def(self < self)
    .def(self == self)
    .def("__hash__", static_cast<size_t (*)(const Security&)>(hash_value))
    .def("__str__", static_cast<string (*)(const Security&)>(&ToString))
    .add_property("symbol", make_function(&Security::GetSymbol,
      return_value_policy<copy_const_reference>()))
    .add_property("market", &Security::GetMarket)
    .add_property("country", &Security::GetCountry);
  def("to_string",
    static_cast<string (*)(const Security&, const MarketDatabase&)>(&ToString));
  def("parse_security",
    static_cast<Security (*)(const string&, const MarketDatabase&)>(
    &ParseSecurity));
  def("parse_security",
    static_cast<Security (*)(const string&)>(&ParseSecurity));
  ExportFixedString<4>();
}

void Nexus::Python::ExportSide() {
  enum_<Side::Type>("Side")
    .value("NONE", Side::NONE)
    .value("ASK", Side::ASK)
    .value("BID", Side::BID);
  ExportEnum<Side>();
  def("pick", static_cast<object& (*)(Side, object&, object&)>(&Pick<object>),
    return_value_policy<return_by_value>());
  def("get_direction", &GetDirection);
  def("get_side", &GetSide);
  def("get_opposite", &GetOpposite);
}

void Nexus::Python::ExportTag() {
  ExportVariant<Tag::Type>();
  class_<Tag>("Tag", init<>())
    .def(init<int, const Tag::Type&>())
    .add_property("key", &Tag::GetKey)
    .add_property("value", make_function(&Tag::GetValue,
      return_value_policy<copy_const_reference>()))
    .def(self == self);
  class_<vector<Tag>>("VectorTag")
    .def(vector_indexing_suite<vector<Tag>>());
}

void Nexus::Python::ExportTimeAndSale() {
  {
    scope outer =
      class_<TimeAndSale>("TimeAndSale", init<>())
        .def(init<const ptime&, Money, Quantity,
          TimeAndSale::Condition, string>())
        .add_property("timestamp", make_getter(&TimeAndSale::m_timestamp,
          return_value_policy<return_by_value>()), make_setter(
          &TimeAndSale::m_timestamp, return_value_policy<return_by_value>()))
        .def_readwrite("price", &TimeAndSale::m_price)
        .def_readwrite("size", &TimeAndSale::m_size)
        .def_readwrite("condition", &TimeAndSale::m_condition)
        .def_readwrite("market_center", &TimeAndSale::m_marketCenter)
        .def(self == self)
        .def(self != self);
    {
      scope outer =
        class_<TimeAndSale::Condition>("Condition", init<>())
          .add_property("type", make_getter(&TimeAndSale::Condition::m_type,
            return_value_policy<return_by_value>()), make_setter(
            &TimeAndSale::Condition::m_type,
            return_value_policy<return_by_value>()))
          .def_readwrite("code", &TimeAndSale::Condition::m_code)
          .def(self == self)
          .def(self != self);
      enum_<TimeAndSale::Condition::Type::Type>("Type")
        .value("NONE", TimeAndSale::Condition::Type::NONE)
        .value("REGULAR", TimeAndSale::Condition::Type::REGULAR)
        .value("OPEN", TimeAndSale::Condition::Type::OPEN)
        .value("CLOSE", TimeAndSale::Condition::Type::CLOSE);
    }
  }
  ExportEnum<TimeAndSale::Condition::Type>();
}

void Nexus::Python::ExportTimeInForce() {
  {
    scope outer =
      class_<TimeInForce>("TimeInForce", init<>())
        .def(init<TimeInForce::Type>())
        .def(init<TimeInForce::Type, const ptime&>())
        .add_property("type", &TimeInForce::GetType)
        .add_property("expiry", make_function(&TimeInForce::GetExpiry,
          return_value_policy<copy_const_reference>()))
        .def(self == self)
        .def(self != self);
      enum_<TimeInForce::Type::Type>("Type")
        .value("NONE", TimeInForce::Type::NONE)
        .value("DAY", TimeInForce::Type::DAY)
        .value("GTC", TimeInForce::Type::GTC)
        .value("OPG", TimeInForce::Type::OPG)
        .value("MOC", TimeInForce::Type::MOC)
        .value("IOC", TimeInForce::Type::IOC)
        .value("FOK", TimeInForce::Type::FOK)
        .value("GTX", TimeInForce::Type::GTX)
        .value("GTD", TimeInForce::Type::GTD);
  }
  ExportEnum<TimeInForce::Type>();
}
