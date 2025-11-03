#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/BoardLotCheck.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  auto make_test_order_fields() {
    auto account = DirectoryEntry::make_account(123, "test");
    auto security = Security("TST", TSX);
    auto currency = CAD;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    return make_limit_order_fields(
      account, security, currency, side, destination, quantity, price);
  }

  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    MarketDataServiceTestEnvironment m_market_data_environment;

    Fixture()
      : m_administration_environment(
          make_administration_service_test_environment(
            m_service_locator_environment)),
        m_market_data_environment(
          make_market_data_service_test_environment(
            m_service_locator_environment, m_administration_environment)) {}
  };
}

TEST_SUITE("BoardLotCheck") {
  TEST_CASE("submit") {
    auto fixture = Fixture();
    auto& feed_client = fixture.m_market_data_environment.get_feed_client();
    auto check = make_board_lot_check(
      fixture.m_market_data_environment.get_registry_client(), DEFAULT_VENUES,
      get_default_time_zone_database());

    SUBCASE("price_over_one_dollar") {
      auto fields = make_test_order_fields();
      auto security = fields.m_security;
      auto bbo = BboQuote(
        make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
          time_from_string("2024-07-18 10:00:00"));
      feed_client.publish(SecurityBboQuote(bbo, security));
      fields.m_quantity = 200;
      auto order_info_valid =
        OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
      REQUIRE_NOTHROW(check->submit(order_info_valid));
      fields.m_quantity = 250;
      auto order_info_invalid =
        OrderInfo(fields, 2, time_from_string("2024-07-18 10:02:00"));
      REQUIRE_THROWS_AS(
        check->submit(order_info_invalid), OrderSubmissionCheckException);
    }

    SUBCASE("price_under_one_dollar") {
      auto fields = make_test_order_fields();
      auto security = fields.m_security;
      auto bbo = BboQuote(
        make_bid(50 * Money::CENT, 100), make_ask(51 * Money::CENT, 100),
        time_from_string("2024-07-18 10:00:00"));
      feed_client.publish(SecurityBboQuote(bbo, security));
      fields.m_quantity = 1500;
      auto order_info_valid =
        OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
      REQUIRE_NOTHROW(check->submit(order_info_valid));
      fields.m_quantity = 1600;
      auto order_info_invalid =
        OrderInfo(fields, 2, time_from_string("2024-07-18 10:02:00"));
      REQUIRE_THROWS_AS(
        check->submit(order_info_invalid), OrderSubmissionCheckException);
    }

    SUBCASE("price_under_ten_cents") {
      auto fields = make_test_order_fields();
      auto security = fields.m_security;
      auto bbo = BboQuote(
        make_bid(9 * Money::CENT, 100), make_ask(10 * Money::CENT, 100),
        time_from_string("2024-07-18 10:00:00"));
      feed_client.publish(SecurityBboQuote(bbo, security));
      fields.m_quantity = 2000;
      auto order_info_valid =
        OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
      REQUIRE_NOTHROW(check->submit(order_info_valid));
      fields.m_quantity = 2500;
      auto order_info_invalid =
        OrderInfo(fields, 2, time_from_string("2024-07-18 10:02:00"));
      REQUIRE_THROWS_AS(
        check->submit(order_info_invalid), OrderSubmissionCheckException);
    }

    SUBCASE("unsupported_venue") {
      auto fields = make_test_order_fields();
      fields.m_security = Security("S32", ASX);
      auto order_info =
        OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
      REQUIRE_NOTHROW(check->submit(order_info));
    }

    SUBCASE("historical_close_price") {
      auto fields = make_test_order_fields();
      auto security = fields.m_security;
      auto previous_close = TimeAndSale(
        time_from_string("2024-07-17 16:00:00"), Money::ONE, 100, {},
        DEFAULT_VENUES.from(TSX).m_market_center);
      fixture.m_market_data_environment.get_data_store().store(
        SequencedSecurityTimeAndSale(
          SecurityTimeAndSale(previous_close, security), Beam::Sequence(1)));
      fields.m_quantity = 300;
      auto order_info_valid =
        OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
      REQUIRE_NOTHROW(check->submit(order_info_valid));
      fields.m_quantity = 350;
      auto order_info_invalid =
        OrderInfo(fields, 2, time_from_string("2024-07-18 10:02:00"));
      REQUIRE_THROWS_AS(
        check->submit(order_info_invalid), OrderSubmissionCheckException);
      auto new_close_timestamp = order_info_invalid.m_timestamp + hours(24);
      auto new_close = TimeAndSale(new_close_timestamp, 10 * Money::CENT, 100,
        {}, DEFAULT_VENUES.from(TSX).m_market_center);
      fixture.m_market_data_environment.get_data_store().store(
        SequencedSecurityTimeAndSale(SecurityTimeAndSale(new_close, security),
        Beam::Sequence(2)));
      auto next_day_timestamp = new_close_timestamp + days(1);
      fields.m_quantity = 500;
      auto next_day_valid_order = OrderInfo(fields, 3, next_day_timestamp);
      REQUIRE_NOTHROW(check->submit(next_day_valid_order));
      fields.m_quantity = 600;
      auto next_day_invalid_order = OrderInfo(fields, 4, next_day_timestamp);
      REQUIRE_THROWS_AS(
        check->submit(next_day_invalid_order), OrderSubmissionCheckException);
    }

    SUBCASE("no_price_available") {
      auto fields = make_test_order_fields();
      auto order_info =
        OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
      REQUIRE_THROWS_AS(
        check->submit(order_info), OrderSubmissionCheckException);
    }
  }
}
