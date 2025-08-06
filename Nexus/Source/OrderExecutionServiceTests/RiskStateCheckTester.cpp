#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/OrderExecutionService/RiskStateCheck.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;

namespace {
  auto make_test_order_fields() {
    auto account = DirectoryEntry::MakeAccount(123, "test");
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
    auto account = DirectoryEntry::MakeAccount(123);
    administration_client.store(account, RiskState(RiskState::Type::DISABLED));
    auto fields = make_test_order_fields();
    auto info = OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
    REQUIRE_THROWS_AS(check->submit(info), OrderSubmissionCheckException);
  }
}
