#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/Evaluator.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <doctest/doctest.h>
#include "Nexus/Queries/BboQuoteAccessor.hpp"
#include "Nexus/Queries/BookQuoteAccessor.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/OrderImbalanceAccessor.hpp"
#include "Nexus/Queries/QuoteAccessor.hpp"
#include "Nexus/Queries/TickerStatusAccessor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("EvaluatorTranslator") {
  TEST_CASE("bbo_quote_timestamp") {
    auto accessor = BboQuoteAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_timestamp());
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 200),
      time_from_string("2026-05-08 09:30:00"));
    auto result = evaluator->eval<ptime>(bbo);
    REQUIRE(result == bbo.m_timestamp);
  }

  TEST_CASE("bbo_quote_bid") {
    auto accessor = BboQuoteAccessor::from_parameter(0);
    auto evaluator = translate<Nexus::EvaluatorTranslator>(accessor.get_bid());
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 200),
      time_from_string("2026-05-08 09:30:00"));
    auto result = evaluator->eval<Quote>(bbo);
    REQUIRE(result == bbo.m_bid);
  }

  TEST_CASE("bbo_quote_ask") {
    auto accessor = BboQuoteAccessor::from_parameter(0);
    auto evaluator = translate<Nexus::EvaluatorTranslator>(accessor.get_ask());
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 200),
      time_from_string("2026-05-08 09:30:00"));
    auto result = evaluator->eval<Quote>(bbo);
    REQUIRE(result == bbo.m_ask);
  }

  TEST_CASE("bbo_quote_bid_price") {
    auto bbo_accessor = BboQuoteAccessor::from_parameter(0);
    auto quote_accessor = QuoteAccessor(bbo_accessor.get_bid());
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(quote_accessor.get_price());
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 200),
      time_from_string("2026-05-08 09:30:00"));
    auto result = evaluator->eval<Money>(bbo);
    REQUIRE(result == Money::ONE);
  }

  TEST_CASE("bbo_quote_ask_size") {
    auto bbo_accessor = BboQuoteAccessor::from_parameter(0);
    auto quote_accessor = QuoteAccessor(bbo_accessor.get_ask());
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(quote_accessor.get_size());
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 200),
      time_from_string("2026-05-08 09:30:00"));
    auto result = evaluator->eval<Quantity>(bbo);
    REQUIRE(result == Quantity(200));
  }

  TEST_CASE("bbo_quote_bid_side") {
    auto bbo_accessor = BboQuoteAccessor::from_parameter(0);
    auto quote_accessor = QuoteAccessor(bbo_accessor.get_bid());
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(quote_accessor.get_side());
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 200),
      time_from_string("2026-05-08 09:30:00"));
    auto result = evaluator->eval<Side>(bbo);
    REQUIRE(result == Side::BID);
  }

  TEST_CASE("quote_price") {
    auto accessor = QuoteAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_price());
    auto quote = make_bid(5 * Money::ONE, 500);
    auto result = evaluator->eval<Money>(quote);
    REQUIRE(result == 5 * Money::ONE);
  }

  TEST_CASE("quote_size") {
    auto accessor = QuoteAccessor::from_parameter(0);
    auto evaluator = translate<Nexus::EvaluatorTranslator>(accessor.get_size());
    auto quote = make_ask(3 * Money::ONE, 300);
    auto result = evaluator->eval<Quantity>(quote);
    REQUIRE(result == Quantity(300));
  }

  TEST_CASE("quote_side") {
    auto accessor = QuoteAccessor::from_parameter(0);
    auto evaluator = translate<Nexus::EvaluatorTranslator>(accessor.get_side());
    auto quote = make_ask(3 * Money::ONE, 300);
    auto result = evaluator->eval<Side>(quote);
    REQUIRE(result == Side::ASK);
  }

  TEST_CASE("bbo_quote_bid_price_filter") {
    auto bbo_accessor = BboQuoteAccessor::from_parameter(0);
    auto quote_accessor = QuoteAccessor(bbo_accessor.get_bid());
    auto filter = quote_accessor.get_price() > ConstantExpression(Money::ONE);
    auto evaluator = translate<Nexus::EvaluatorTranslator>(filter);
    auto bbo_above =
      BboQuote(make_bid(2 * Money::ONE, 100), make_ask(3 * Money::ONE, 200),
        time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<bool>(bbo_above));
    auto bbo_below =
      BboQuote(make_bid(Money::CENT, 100), make_ask(Money::ONE, 200),
        time_from_string("2026-05-08 09:30:01"));
    REQUIRE(!evaluator->eval<bool>(bbo_below));
  }

  TEST_CASE("book_quote_mpid") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto evaluator = translate<Nexus::EvaluatorTranslator>(accessor.get_mpid());
    auto quote = BookQuote("MM01", true, Venues::TSX, make_bid(Money::ONE, 100),
      time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<std::string>(quote) == "MM01");
  }

  TEST_CASE("book_quote_is_primary_mpid") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.is_primary_mpid());
    auto quote = BookQuote("MM01", true, Venues::TSX, make_bid(Money::ONE, 100),
      time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<bool>(quote));
  }

  TEST_CASE("book_quote_venue") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_venue());
    auto quote = BookQuote("MM01", true, Venues::TSX, make_bid(Money::ONE, 100),
      time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<Venue>(quote) == Venues::TSX);
  }

  TEST_CASE("book_quote_timestamp") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_timestamp());
    auto timestamp = time_from_string("2026-05-08 09:30:00");
    auto quote = BookQuote(
      "MM01", true, Venues::TSX, make_bid(Money::ONE, 100), timestamp);
    REQUIRE(evaluator->eval<ptime>(quote) == timestamp);
  }

  TEST_CASE("book_quote_quote_price") {
    auto book_accessor = BookQuoteAccessor::from_parameter(0);
    auto quote_accessor = QuoteAccessor(book_accessor.get_quote());
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(quote_accessor.get_price());
    auto quote = BookQuote("MM01", true, Venues::TSX,
      make_bid(5 * Money::ONE, 500), time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<Money>(quote) == 5 * Money::ONE);
  }

  TEST_CASE("book_quote_mpid_filter") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto filter = accessor.get_mpid() ==
      ConstantExpression(std::string("MM01"));
    auto evaluator = translate<Nexus::EvaluatorTranslator>(filter);
    auto match = BookQuote("MM01", true, Venues::TSX, make_bid(Money::ONE, 100),
      time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<bool>(match));
    auto no_match = BookQuote("MM02", false, Venues::TSX,
      make_bid(Money::ONE, 100), time_from_string("2026-05-08 09:30:01"));
    REQUIRE(!evaluator->eval<bool>(no_match));
  }

  TEST_CASE("order_imbalance_side") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_side());
    auto imbalance = OrderImbalance(parse_ticker("A.TSX"), Side::BID, 1000,
      Money::ONE, time_from_string("2026-05-08 15:50:00"));
    REQUIRE(evaluator->eval<Side>(imbalance) == Side::BID);
  }

  TEST_CASE("order_imbalance_size") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_size());
    auto imbalance = OrderImbalance(parse_ticker("A.TSX"), Side::ASK, 500,
      2 * Money::ONE, time_from_string("2026-05-08 15:50:00"));
    REQUIRE(evaluator->eval<Quantity>(imbalance) == Quantity(500));
  }

  TEST_CASE("order_imbalance_reference_price") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_reference_price());
    auto imbalance = OrderImbalance(parse_ticker("A.TSX"), Side::BID, 1000,
      5 * Money::ONE, time_from_string("2026-05-08 15:50:00"));
    REQUIRE(evaluator->eval<Money>(imbalance) == 5 * Money::ONE);
  }

  TEST_CASE("order_imbalance_timestamp") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_timestamp());
    auto timestamp = time_from_string("2026-05-08 15:50:00");
    auto imbalance = OrderImbalance(
      parse_ticker("A.TSX"), Side::BID, 1000, Money::ONE, timestamp);
    REQUIRE(evaluator->eval<ptime>(imbalance) == timestamp);
  }

  TEST_CASE("order_imbalance_size_filter") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto filter = accessor.get_size() > ConstantExpression(Quantity(500));
    auto evaluator = translate<Nexus::EvaluatorTranslator>(filter);
    auto large = OrderImbalance(parse_ticker("A.TSX"), Side::BID, 1000,
      Money::ONE, time_from_string("2026-05-08 15:50:00"));
    REQUIRE(evaluator->eval<bool>(large));
    auto small = OrderImbalance(parse_ticker("A.TSX"), Side::ASK, 100,
      Money::ONE, time_from_string("2026-05-08 15:50:01"));
    REQUIRE(!evaluator->eval<bool>(small));
  }

  TEST_CASE("ticker_status_state") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_state());
    auto status =
      TickerStatus(Venues::TSX, "Authorized", TickerStatus::Flag::IS_CONTINUOUS,
        time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<std::string>(status) == "Authorized");
  }

  TEST_CASE("ticker_status_venue") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_venue());
    auto status =
      TickerStatus(Venues::TSX, "Authorized", TickerStatus::Flag::IS_CONTINUOUS,
        time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<Venue>(status) == Venues::TSX);
  }

  TEST_CASE("ticker_status_flags") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_flags());
    auto status =
      TickerStatus(Venues::TSX, "Authorized", TickerStatus::Flag::IS_CONTINUOUS,
        time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<int>(status) ==
      static_cast<int>(TickerStatus::Flag::IS_CONTINUOUS));
  }

  TEST_CASE("ticker_status_timestamp") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto evaluator =
      translate<Nexus::EvaluatorTranslator>(accessor.get_timestamp());
    auto timestamp = time_from_string("2026-05-08 09:30:00");
    auto status = TickerStatus(
      Venues::TSX, "Authorized", TickerStatus::Flag::IS_CONTINUOUS, timestamp);
    REQUIRE(evaluator->eval<ptime>(status) == timestamp);
  }

  TEST_CASE("ticker_status_state_filter") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto filter =
      accessor.get_state() == ConstantExpression(std::string("Authorized"));
    auto evaluator = translate<Nexus::EvaluatorTranslator>(filter);
    auto match =
      TickerStatus(Venues::TSX, "Authorized", TickerStatus::Flag::IS_CONTINUOUS,
        time_from_string("2026-05-08 09:30:00"));
    REQUIRE(evaluator->eval<bool>(match));
    auto no_match = TickerStatus(
      Venues::TSX, "AuthorizedHalted", TickerStatus::Flag::IS_ACCEPTING_CANCELS,
      time_from_string("2026-05-08 10:00:00"));
    REQUIRE(!evaluator->eval<bool>(no_match));
  }
}
