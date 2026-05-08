#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/Evaluator.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <doctest/doctest.h>
#include "Nexus/Queries/BboQuoteAccessor.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/QuoteAccessor.hpp"

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
    REQUIRE(evaluator->eval<bool>(bbo_above) == true);
    auto bbo_below =
      BboQuote(make_bid(Money::CENT, 100), make_ask(Money::ONE, 200),
        time_from_string("2026-05-08 09:30:01"));
    REQUIRE(evaluator->eval<bool>(bbo_below) == false);
  }
}
