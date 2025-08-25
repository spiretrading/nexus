#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskState.hpp"

using namespace Beam;
using namespace Nexus;
using namespace boost::posix_time;

TEST_SUITE("RiskState") {
  TEST_CASE("constructor") {
    auto default_state = RiskState();
    REQUIRE(default_state.m_type == RiskState::Type::ACTIVE);
    REQUIRE(default_state.m_expiry == boost::posix_time::pos_infin);
    auto close_orders_state = RiskState(RiskState::Type::CLOSE_ORDERS);
    REQUIRE(close_orders_state.m_type == RiskState::Type::CLOSE_ORDERS);
    REQUIRE(close_orders_state.m_expiry == boost::posix_time::pos_infin);
    auto expiry = time_from_string("2025-08-14 13:00:05");
    auto expired_state = RiskState(RiskState::Type::CLOSE_ORDERS, expiry);
    REQUIRE(expired_state.m_type == RiskState::Type::CLOSE_ORDERS);
    REQUIRE(expired_state.m_expiry == expiry);
  }

  TEST_CASE("stream_type") {
    auto out = std::ostringstream();
    out << RiskState::Type::ACTIVE;
    REQUIRE(out.str() == "ACTIVE");
    out.str("");
    out << RiskState::Type::CLOSE_ORDERS;
    REQUIRE(out.str() == "CLOSE_ORDERS");
    out.str("");
    out << RiskState::Type::DISABLED;
    REQUIRE(out.str() == "DISABLED");
    out.str("");
    out << static_cast<RiskState::Type>(123);
    REQUIRE(out.str() == "NONE");
  }

  TEST_CASE("stream") {
    auto expiry = time_from_string("2025-08-14 13:00:05");
    auto state = RiskState(RiskState::Type::CLOSE_ORDERS, expiry);
    auto out = std::ostringstream();
    out << state;
    REQUIRE(out.str() == "(CLOSE_ORDERS 2025-Aug-14 13:00:05)");
    SUBCASE("shuttle") {
      Beam::Serialization::Tests::TestRoundTripShuttle(state);
    }
  }
}
