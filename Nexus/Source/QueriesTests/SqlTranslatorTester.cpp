#include <Beam/Queries/StandardValues.hpp>
#include <Viper/Sqlite3/QueryBuilder.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/Queries/AccountModificationRequestAccessor.hpp"
#include "Nexus/Queries/BboQuoteAccessor.hpp"
#include "Nexus/Queries/BookQuoteAccessor.hpp"
#include "Nexus/Queries/OrderImbalanceAccessor.hpp"
#include "Nexus/Queries/QuoteAccessor.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"
#include "Nexus/Queries/TickerStatusAccessor.hpp"
#include "Nexus/Queries/TimeAndSaleAccessor.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  auto translate(const std::string& table, const Expression& expression) {
    auto translator = Nexus::SqlTranslator(table, expression);
    auto translation = translator.make();
    auto query = std::string();
    Viper::Sqlite3::build_query(translation, query);
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
    auto expression = accessor.get_venue() == ConstantExpression(Venue("TSE"));
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

  TEST_CASE("account_modification_request_status") {
    auto accessor = AccountModificationRequestAccessor::from_parameter(0);
    auto expression = accessor.get_status() == ConstantExpression(
      static_cast<int>(AccountModificationRequest::Status::GRANTED));
    REQUIRE(translate("account_modification_requests", expression) ==
      "(status = 4)");
  }

  TEST_CASE("account_modification_request_last_update_timestamp") {
    auto accessor = AccountModificationRequestAccessor::from_parameter(0);
    auto expression = accessor.get_last_update_timestamp() >=
      ConstantExpression(time_from_string("2026-05-08 09:30:00"));
    REQUIRE(translate("account_modification_requests", expression) ==
      "(last_update_timestamp >= 1778232600000)");
  }

  TEST_CASE("scaled_arithmetic") {
    SUBCASE("product") {
      auto parameters = std::vector<Expression>{
        ConstantExpression(Quantity(3)), ConstantExpression(Quantity(4))};
      auto expression =
        FunctionExpression(MULTIPLICATION_NAME, typeid(Quantity), parameters);
      REQUIRE(translate("order_imbalances", expression) ==
        "((3000000.000000 * 4000000.000000) / 1000000)");
    }

    SUBCASE("quotient") {
      auto parameters = std::vector<Expression>{
        ConstantExpression(Quantity(12)), ConstantExpression(Quantity(4))};
      auto expression =
        FunctionExpression(DIVISION_NAME, typeid(Quantity), parameters);
      REQUIRE(translate("order_imbalances", expression) ==
        "((12000000.000000 / 4000000.000000) * 1000000)");
    }

    SUBCASE("sum") {
      auto parameters = std::vector<Expression>{
        ConstantExpression(Money::ONE), ConstantExpression(Money::ONE)};
      auto expression =
        FunctionExpression(ADDITION_NAME, typeid(Money), parameters);
      REQUIRE(translate("order_imbalances", expression) ==
        "(1000000.000000 + 1000000.000000)");
    }
  }

  TEST_CASE("mismatched_member_type_throws") {
    SUBCASE("member") {
      auto member = MemberAccessExpression("timestamp", typeid(int),
        ParameterExpression(0, typeid(AccountModificationRequest)));
      auto expression = member == ConstantExpression(7);
      REQUIRE_THROWS_AS(translate("account_modification_requests", expression),
        ExpressionTranslationException);
    }

    SUBCASE("operand") {
      auto operand =
        FunctionExpression(EQUALS_NAME, typeid(AccountModificationRequest),
          {ConstantExpression(0), ConstantExpression(0)});
      auto member = MemberAccessExpression("id", typeid(int), operand);
      auto expression = member == ConstantExpression(7);
      REQUIRE_THROWS_AS(translate("account_modification_requests", expression),
        ExpressionTranslationException);
    }
  }

  TEST_CASE("mixed_operand_types") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto money = ConstantExpression(Money::ONE);
    auto quantity = ConstantExpression(Quantity(3));
    SUBCASE("quantity_and_scalar") {
      REQUIRE(translate("order_imbalances",
        accessor.get_size() > ConstantExpression(500)) ==
          "(order_imbalances.size > (500 * 1000000))");
      REQUIRE(translate("order_imbalances",
        ConstantExpression(500) < accessor.get_size()) ==
          "((500 * 1000000) < order_imbalances.size)");
      REQUIRE(translate("order_imbalances",
        accessor.get_size() >= ConstantExpression(2.5)) ==
          "(order_imbalances.size >= (2.500000 * 1000000))");
    }

    SUBCASE("quantity_arithmetic") {
      REQUIRE(translate("order_imbalances",
        accessor.get_size() + ConstantExpression(5)) ==
          "(order_imbalances.size + (5 * 1000000))");
      REQUIRE(translate("order_imbalances",
        ConstantExpression(5) + accessor.get_size()) ==
          "((5 * 1000000) + order_imbalances.size)");
      REQUIRE(translate("order_imbalances",
        accessor.get_size() * ConstantExpression(5)) ==
          "(order_imbalances.size * 5)");
      REQUIRE(translate("order_imbalances",
        ConstantExpression(5) / accessor.get_size()) ==
          "((5 / order_imbalances.size) * 1000000000000)");
    }

    SUBCASE("money_ratio") {
      auto expression = money / ConstantExpression(2 * Money::ONE);
      REQUIRE(expression.get_type() == typeid(double));
      REQUIRE(translate("order_imbalances", expression) ==
        "(1000000.000000 / 2000000.000000)");
    }

    SUBCASE("max_and_min") {
      REQUIRE(translate("order_imbalances",
        max(accessor.get_size(), ConstantExpression(500))) ==
          "MAX(order_imbalances.size, (500 * 1000000))");
      REQUIRE(translate("order_imbalances",
        min(ConstantExpression(500), accessor.get_size())) ==
          "MIN((500 * 1000000), order_imbalances.size)");
    }

    SUBCASE("incompatible") {
      REQUIRE_THROWS_AS(
        translate("order_imbalances", money > ConstantExpression(2)),
        ExpressionTranslationException);
      REQUIRE_THROWS_AS(translate("order_imbalances", money > quantity),
        ExpressionTranslationException);
      REQUIRE_THROWS_AS(translate("order_imbalances", quantity * money),
        ExpressionTranslationException);
      REQUIRE_THROWS_AS(
        translate("order_imbalances", money / ConstantExpression(2)),
        ExpressionTranslationException);
    }
  }
}
