#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  auto ABX() {
    return parse_ticker("ABX.TSX");
  }

  auto TD() {
    return parse_ticker("TD.TSX");
  }

  auto TSX() {
    return Scope(DefaultVenues::TSX);
  }
}

TEST_SUITE("OrderTaskArguments") {
  TEST_CASE("find_exact_ticker_argument_match") {
    auto arguments = ArrayListModel<OrderTaskArguments>();
    arguments.push({"Test1", ABX(), DefaultDestinations::TSX,
      OrderType::LIMIT, Side::BID, QuantitySetting::ADJUSTABLE,
      TimeInForce::Type::DAY, {}, Qt::Key_F2});
    arguments.push({"Test2", TD(), DefaultDestinations::CHIX,
      OrderType::MARKET, Side::ASK, QuantitySetting::ADJUSTABLE,
      TimeInForce::Type::DAY, {}, Qt::Key_F3});
    auto match_test_1 = find_order_task_arguments(arguments, ABX(), Qt::Key_F2);
    REQUIRE(match_test_1.is_initialized());
    REQUIRE(match_test_1->m_key == Qt::Key_F2);
    REQUIRE((!find_order_task_arguments(arguments, TD(), Qt::Key_F2)));
    REQUIRE((!find_order_task_arguments(arguments, ABX(), Qt::Key_F3)));
    REQUIRE((!find_order_task_arguments(arguments, TSX(), Qt::Key_F2)));
    REQUIRE((!find_order_task_arguments(
      arguments, DefaultCountries::CA, Qt::Key_F2)));
    auto match_test_2 = find_order_task_arguments(arguments, TD(), Qt::Key_F3);
    REQUIRE(match_test_2.is_initialized());
    REQUIRE(match_test_2->m_key == Qt::Key_F3);
    REQUIRE((!find_order_task_arguments(arguments, ABX(), Qt::Key_F3)));
    REQUIRE((!find_order_task_arguments(arguments, TD(), Qt::Key_F2)));
    REQUIRE((!find_order_task_arguments(arguments, TSX(), Qt::Key_F3)));
    REQUIRE((!find_order_task_arguments(
      arguments, DefaultCountries::CA, Qt::Key_F3)));
    REQUIRE((!find_order_task_arguments(arguments, ABX(), Qt::Key_F4)));
  }

  TEST_CASE("find_venue_scope_argument_match") {
    auto arguments = ArrayListModel<OrderTaskArguments>();
    arguments.push({"Test1", TSX(), DefaultDestinations::TSX,
      OrderType::LIMIT, Side::BID, QuantitySetting::ADJUSTABLE,
      TimeInForce::Type::DAY, {}, Qt::Key_F2});
    arguments.push({"Test2", TD(), DefaultDestinations::CHIX,
      OrderType::MARKET, Side::ASK, QuantitySetting::ADJUSTABLE,
      TimeInForce::Type::DAY, {}, Qt::Key_F2});
    auto match_test_1_ticker =
      find_order_task_arguments(arguments, ABX(), Qt::Key_F2);
    REQUIRE(match_test_1_ticker.is_initialized());
    REQUIRE(match_test_1_ticker->m_key == Qt::Key_F2);
    REQUIRE(match_test_1_ticker->m_destination == DefaultDestinations::TSX);
    auto match_test_1_venue =
      find_order_task_arguments(arguments, TSX(), Qt::Key_F2);
    REQUIRE(match_test_1_venue.is_initialized());
    REQUIRE(match_test_1_venue->m_key == Qt::Key_F2);
    REQUIRE(match_test_1_venue->m_destination == DefaultDestinations::TSX);
    auto match_test_2_ticker =
      find_order_task_arguments(arguments, TD(), Qt::Key_F2);
    REQUIRE(match_test_2_ticker.is_initialized());
    REQUIRE(match_test_2_ticker->m_key == Qt::Key_F2);
    REQUIRE(match_test_2_ticker->m_destination == DefaultDestinations::CHIX);
  }
}
