#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/OrderImbalance.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("OrderImbalance") {
  TEST_CASE("stream") {
    auto imbalance = OrderImbalance(parse_ticker("ABC.TSX"), Side::BID, 42,
      Money(12345), time_from_string("2020-01-02 03:04:05"));
    auto expected_security = to_string(imbalance.m_ticker);
    auto expected_side = to_string(imbalance.m_side);
    auto expected_size = to_string(imbalance.m_size);
    auto expected_price = to_string(imbalance.m_reference_price);
    auto expected_timestamp = to_string(imbalance.m_timestamp);
    auto expected_output = "(" + expected_security + " " + expected_side +
      " " + expected_size + " " + expected_price + " " + expected_timestamp +
      ")";
    REQUIRE(to_string(imbalance) == expected_output);
    test_round_trip_shuttle(imbalance);
  }
}
