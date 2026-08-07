#include <Beam/Queries/StandardValues.hpp>
#include <doctest/doctest.h>
#include "Nexus/Queries/BboQuoteAccessor.hpp"
#include "Nexus/Queries/BookQuoteAccessor.hpp"
#include "Nexus/Queries/OrderImbalanceAccessor.hpp"
#include "Nexus/Queries/QuoteAccessor.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"
#include "Nexus/Queries/TickerStatusAccessor.hpp"
#include "Nexus/Queries/TimeAndSaleAccessor.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  auto translate(const std::string& table, const Expression& expression) {
    auto translator = Nexus::SqlTranslator(table, expression);
    auto translation = translator.make();
    auto query = std::string();
    translation.append_query(query);
    return query;
  }
}

TEST_SUITE("SqlTranslator") {
  TEST_CASE("query_order_fields") {
    auto info_parameter_expression = ParameterExpression(0, typeid(OrderInfo));
    auto fields_access_expression = MemberAccessExpression(
      "fields", typeid(OrderFields), info_parameter_expression);
    auto ticker_access_expression = MemberAccessExpression(
      "ticker", typeid(Ticker), fields_access_expression);
    auto venue_access_expression = MemberAccessExpression(
      "venue", typeid(std::string), ticker_access_expression);
    auto equal_expression = "XTSX" == venue_access_expression;
    REQUIRE(translate("submissions", equal_expression) == "(\"XTSX\" = venue)");
  }

  TEST_CASE("query_bbo_quote_bid_price") {
    auto bbo_accessor = BboQuoteAccessor::from_parameter(0);
    auto quote_accessor = QuoteAccessor(bbo_accessor.get_bid());
    auto expression =
      quote_accessor.get_price() > ConstantExpression(Money::ONE);
    REQUIRE(translate("bbo_quotes", expression) ==
      "(bbo_quotes.bid.price > 1000000.000000)");
  }

  TEST_CASE("query_bbo_quote_ask_size") {
    auto bbo_accessor = BboQuoteAccessor::from_parameter(0);
    auto quote_accessor = QuoteAccessor(bbo_accessor.get_ask());
    auto expression =
      quote_accessor.get_size() > ConstantExpression(Quantity(100));
    REQUIRE(translate("bbo_quotes", expression) ==
      "(bbo_quotes.ask.size > 100000000.000000)");
  }

  TEST_CASE("query_book_quote_mpid") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto expression =
      accessor.get_mpid() == ConstantExpression(std::string("MM01"));
    REQUIRE(
      translate("book_quotes", expression) == "(book_quotes.mpid = \"MM01\")");
  }

  TEST_CASE("query_book_quote_venue") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto expression =
      accessor.get_venue() == ConstantExpression(std::string("TSE"));
    REQUIRE(translate("book_quotes", expression) ==
      "(book_quotes.quote_venue = \"TSE\")");
  }

  TEST_CASE("query_book_quote_quote_price") {
    auto book_accessor = BookQuoteAccessor::from_parameter(0);
    auto quote_accessor = QuoteAccessor(book_accessor.get_quote());
    auto expression =
      quote_accessor.get_price() > ConstantExpression(Money::ONE);
    REQUIRE(translate("book_quotes", expression) == "(price > 1000000.000000)");
  }

  TEST_CASE("query_order_imbalance_size") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto expression = accessor.get_size() > ConstantExpression(Quantity(500));
    REQUIRE(translate("order_imbalances", expression) ==
      "(order_imbalances.size > 500000000.000000)");
  }

  TEST_CASE("query_order_imbalance_reference_price") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto expression =
      accessor.get_reference_price() > ConstantExpression(Money::ONE);
    REQUIRE(translate("order_imbalances", expression) ==
      "(order_imbalances.price > 1000000.000000)");
  }

  TEST_CASE("query_ticker_status_state") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto expression =
      accessor.get_state() == ConstantExpression(std::string("Authorized"));
    REQUIRE(translate("ticker_statuses", expression) ==
      "(ticker_statuses.state = \"Authorized\")");
  }

  TEST_CASE("query_ticker_status_venue") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto expression =
      accessor.get_venue() == ConstantExpression(std::string("TSE"));
    REQUIRE(translate("ticker_statuses", expression) ==
      "(ticker_statuses.status_venue = \"TSE\")");
  }

  TEST_CASE("query_time_and_sale_market_center") {
    auto accessor = TimeAndSaleAccessor::from_parameter(0);
    auto expression =
      accessor.get_market_center() == ConstantExpression(std::string("TSX"));
    REQUIRE(translate("time_and_sales", expression) ==
      "(time_and_sales.market = \"TSX\")");
  }

  TEST_CASE("constant_side") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto expression =
      accessor.get_side() == ConstantExpression(Side(Side::BID));
    REQUIRE(translate("order_imbalances", expression) ==
      "(order_imbalances.side = 1)");
  }

  TEST_CASE("constant_venue") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto expression = accessor.get_venue() == ConstantExpression(Venue("TSE"));
    REQUIRE(translate("book_quotes", expression) ==
      "(book_quotes.quote_venue = \"TSE\")");
  }

  TEST_CASE("query_order_ids") {
    auto ids = std::vector<OrderId>{13, 31};
    auto expression = make_order_id_filter(ids);
    REQUIRE(translate("submissions", expression) ==
      "((submissions.order_id = 13) OR (submissions.order_id = 31))");
  }
}
