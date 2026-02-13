#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderFields.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("OrderFields") {
  TEST_CASE("default_constructor") {
    auto fields = OrderFields();
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(!fields.m_ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::NONE);
    REQUIRE(fields.m_side == Side::NONE);
    REQUIRE(fields.m_destination.empty());
    REQUIRE(fields.m_quantity == 0);
    REQUIRE(fields.m_price == Money::ZERO);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::GTC);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("constructor") {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto currency = USD;
    auto type = OrderType::LIMIT;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto time_in_force = TimeInForce::Type::FOK;
    auto additional_fields = std::vector{Tag(1, 123)};
    auto fields = OrderFields(account, ticker, currency, type, side,
      destination, quantity, price, time_in_force, additional_fields);
    REQUIRE(fields.m_account == account);
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(fields.m_currency == currency);
    REQUIRE(fields.m_type == type);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == price);
    REQUIRE(fields.m_time_in_force == time_in_force);
    REQUIRE(fields.m_additional_fields == additional_fields);
  }

  TEST_CASE("make_limit_order_fields_all_parameters") {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto currency = USD;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto fields = make_limit_order_fields(
      account, ticker, currency, side, destination, quantity, price);
    REQUIRE(fields.m_account == account);
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(fields.m_currency == currency);
    REQUIRE(fields.m_type == OrderType::LIMIT);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == price);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_limit_order_fields_no_account") {
    auto ticker = parse_ticker("TST.TSX");
    auto currency = USD;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto fields = make_limit_order_fields(
      ticker, currency, side, destination, quantity, price);
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(fields.m_currency == currency);
    REQUIRE(fields.m_type == OrderType::LIMIT);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == price);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_limit_order_fields_no_currency") {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto fields = make_limit_order_fields(
      account, ticker, side, destination, quantity, price);
    REQUIRE(fields.m_account == account);
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::LIMIT);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == price);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_limit_order_fields_no_account_or_currency") {
    auto ticker = parse_ticker("TST.TSX");
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto fields =
      make_limit_order_fields(ticker, side, destination, quantity, price);
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::LIMIT);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == price);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_limit_order_fields_no_destination") {
    auto ticker = parse_ticker("TST.TSX");
    auto currency = USD;
    auto side = Side::BID;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto fields =
      make_limit_order_fields(ticker, currency, side, quantity, price);
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(fields.m_currency == currency);
    REQUIRE(fields.m_type == OrderType::LIMIT);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination.empty());
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == price);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_limit_order_fields_no_currency_or_destination") {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto side = Side::BID;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto fields =
      make_limit_order_fields(account, ticker, side, quantity, price);
    REQUIRE(fields.m_account == account);
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::LIMIT);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination.empty());
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == price);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_limit_order_fields_minimal_parameters") {
    auto ticker = parse_ticker("TST.TSX");
    auto side = Side::BID;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto fields = make_limit_order_fields(ticker, side, quantity, price);
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::LIMIT);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination.empty());
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == price);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_market_order_fields_all_parameters") {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto currency = USD;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto fields = make_market_order_fields(
      account, ticker, currency, side, destination, quantity);
    REQUIRE(fields.m_account == account);
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(fields.m_currency == currency);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == Money::ZERO);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_market_order_fields_no_account") {
    auto ticker = parse_ticker("TST.TSX");
    auto currency = USD;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto fields =
      make_market_order_fields(ticker, currency, side, destination, quantity);
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(fields.m_currency == currency);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == Money::ZERO);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_market_order_fields_no_currency") {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto fields =
      make_market_order_fields(account, ticker, side, destination, quantity);
    REQUIRE(fields.m_account == account);
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == Money::ZERO);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_market_order_fields_no_account_or_currency") {
    auto ticker = parse_ticker("TST.TSX");
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto fields =
      make_market_order_fields(ticker, side, destination, quantity);
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination == destination);
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == Money::ZERO);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_market_order_fields_no_destination") {
    auto ticker = parse_ticker("TST.TSX");
    auto currency = USD;
    auto side = Side::BID;
    auto quantity = Quantity(100);
    auto fields = make_market_order_fields(ticker, currency, side, quantity);
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(fields.m_currency == currency);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination.empty());
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == Money::ZERO);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_market_order_fields_no_currency_or_destination") {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto side = Side::BID;
    auto quantity = Quantity(100);
    auto fields = make_market_order_fields(account, ticker, side, quantity);
    REQUIRE(fields.m_account == account);
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination.empty());
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == Money::ZERO);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("make_market_order_fields_minimal_parameters") {
    auto ticker = parse_ticker("TST.TSX");
    auto side = Side::BID;
    auto quantity = Quantity(100);
    auto fields = make_market_order_fields(ticker, side, quantity);
    REQUIRE(fields.m_account == DirectoryEntry());
    REQUIRE(fields.m_ticker == ticker);
    REQUIRE(!fields.m_currency);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_destination.empty());
    REQUIRE(fields.m_quantity == quantity);
    REQUIRE(fields.m_price == Money::ZERO);
    REQUIRE(fields.m_time_in_force == TimeInForce::Type::DAY);
    REQUIRE(fields.m_additional_fields.empty());
  }

  TEST_CASE("less_than_operator") {
    auto fields1 = OrderFields();
    fields1.m_type = OrderType::LIMIT;
    fields1.m_side = Side::BID;
    fields1.m_price = Money::ONE;
    auto fields2 = OrderFields();
    fields2.m_type = OrderType::MARKET;
    fields2.m_side = Side::BID;
    fields2.m_price = Money::ONE;
    REQUIRE(fields2 < fields1);
    auto fields3 = OrderFields();
    fields3.m_type = OrderType::LIMIT;
    fields3.m_side = Side::BID;
    fields3.m_price = 2 * Money::ONE;
    REQUIRE(fields1 < fields3);
    auto fields4 = OrderFields();
    fields4.m_type = OrderType::LIMIT;
    fields4.m_side = Side::ASK;
    fields4.m_price = Money::ONE;
    REQUIRE(!(fields1 < fields4));
    auto fields5 = OrderFields();
    fields5.m_type = OrderType::LIMIT;
    fields5.m_side = Side::ASK;
    fields5.m_price = 2 * Money::ONE;
    REQUIRE(fields5 < fields4);
  }

  TEST_CASE("has_field") {
    auto fields = OrderFields();
    fields.m_additional_fields.emplace_back(12, "abc");
    fields.m_additional_fields.emplace_back(13, 123.456);
    REQUIRE(has_field(fields, Tag(12, "abc")));
    REQUIRE(has_field(fields, Tag(13, 123.456)));
    REQUIRE(!has_field(fields, Tag(14, "cde")));
    REQUIRE(!has_field(fields, Tag(12, "cde")));
  }

  TEST_CASE("find_field") {
    auto fields = OrderFields();
    fields.m_additional_fields.emplace_back(12, "abc");
    fields.m_additional_fields.emplace_back(13, 123.456);
    auto tag1 = find_field(fields, 12);
    REQUIRE(tag1);
    REQUIRE(*tag1 == Tag(12, "abc"));
    auto tag2 = find_field(fields, 13);
    REQUIRE(tag2);
    REQUIRE(*tag2 == Tag(13, 123.456));
    auto tag3 = find_field(fields, 14);
    REQUIRE(!tag3);
  }

  TEST_CASE("stream") {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto currency = USD;
    auto type = OrderType::LIMIT;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    auto time_in_force = TimeInForce::Type::FOK;
    auto additional_fields = std::vector{Tag(1, 123)};
    auto fields = OrderFields(account, ticker, currency, type, side,
      destination, quantity, price, time_in_force, additional_fields);
    REQUIRE(to_string(fields) ==
      "((ACCOUNT 123 test) TST.TSX USD LIMIT BID TSX 100 1.00 FOK [(1 123)])");
    test_round_trip_shuttle(fields);
  }
}
