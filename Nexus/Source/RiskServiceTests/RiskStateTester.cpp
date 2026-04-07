#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskState.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace boost::posix_time;

TEST_SUITE("RiskState") {
  TEST_CASE("constructor") {
    auto default_state = RiskState();
    REQUIRE(default_state.m_type == RiskState::Type::ACTIVE);
    REQUIRE(default_state.m_expiry == pos_infin);
    auto close_orders_state = RiskState(RiskState::Type::CLOSE_ORDERS);
    REQUIRE(close_orders_state.m_type == RiskState::Type::CLOSE_ORDERS);
    REQUIRE(close_orders_state.m_expiry == pos_infin);
    auto expiry = time_from_string("2025-08-14 13:00:05");
    auto expired_state = RiskState(RiskState::Type::CLOSE_ORDERS, expiry);
    REQUIRE(expired_state.m_type == RiskState::Type::CLOSE_ORDERS);
    REQUIRE(expired_state.m_expiry == expiry);
  }

  TEST_CASE("stream") {
    auto state = RiskState(
      RiskState::Type::CLOSE_ORDERS, time_from_string("2025-08-14 13:00:05"));
    REQUIRE(to_string(state) == "(CLOSE_ORDERS 2025-Aug-14 13:00:05)");
    test_round_trip_shuttle(state);
  }
}
