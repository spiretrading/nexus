#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionService/RiskStateCheck.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

namespace {
  auto make_test_order_fields() {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto currency = CAD;
    auto side = Side::BID;
    auto destination = Destinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    return make_limit_order_fields(
      account, ticker, currency, side, destination, quantity, price);
  }

  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;

    Fixture()
      : m_administration_environment(
          make_administration_service_test_environment(
            m_service_locator_environment)) {}
  };
}

TEST_SUITE("RiskStateCheck") {
  TEST_CASE("closed_orders_rejection") {
    auto fixture = Fixture();
    auto& administration_client =
      fixture.m_administration_environment.get_client();
    auto check = make_risk_state_check(administration_client);
    auto account = DirectoryEntry::make_account(123);
    administration_client.store(account, RiskState::Type::DISABLED);
    auto fields = make_test_order_fields();
    auto info = OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
    REQUIRE_THROWS_AS(check->submit(info), OrderSubmissionCheckException);
  }

  TEST_CASE("restore_seeds_positions") {
    auto fixture = Fixture();
    auto& administration_client =
      fixture.m_administration_environment.get_client();
    auto check = make_risk_state_check(administration_client);
    auto account = DirectoryEntry::make_account(123);
    administration_client.store(account, RiskState::Type::DISABLED);
    auto ticker = parse_ticker("TST.TSX");
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories.push_back(Inventory(
      Position(ticker, CAD, 100, 100 * Money::ONE), Money::ZERO, Money::ZERO,
      100, 1));
    check->restore(account, snapshot, {});
    auto closing_fields = make_limit_order_fields(
      account, ticker, CAD, Side::ASK, Destinations::TSX, 100, Money::ONE);
    auto closing_info =
      OrderInfo(closing_fields, 1, time_from_string("2024-07-18 10:01:00"));
    REQUIRE_NOTHROW(check->submit(closing_info));
    auto opening_fields = make_limit_order_fields(
      account, ticker, CAD, Side::BID, Destinations::TSX, 100, Money::ONE);
    auto opening_info =
      OrderInfo(opening_fields, 2, time_from_string("2024-07-18 10:02:00"));
    REQUIRE_THROWS_AS(
      check->submit(opening_info), OrderSubmissionCheckException);
  }
}
