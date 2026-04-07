#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TickerInfo.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("TickerInfo") {
  TEST_CASE("stream") {
    auto info =
      TickerInfo(Ticker("ABC", TSX), "Test Company", "Technology", 100);
    auto expected = std::ostringstream();
    expected << '(' << info.m_ticker << ' ' << info.m_name << ' '<<
      info.m_sector << ' ' << info.m_board_lot << ')';
    REQUIRE(to_string(info) == expected.str());
    test_round_trip_shuttle(info);
  }
}
