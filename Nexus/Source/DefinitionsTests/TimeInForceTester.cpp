#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TimeInForce.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("TimeInForce") {
  TEST_CASE("constructor") {
    auto tif = TimeInForce();
    REQUIRE((tif.get_type() == TimeInForce::Type::NONE));
    REQUIRE(tif.get_expiry() == not_a_date_time);
  }

  TEST_CASE("construct_type") {
    auto tif = TimeInForce(TimeInForce::Type::GTC);
    REQUIRE((tif.get_type() == TimeInForce::Type::GTC));
    REQUIRE(tif.get_expiry() == not_a_date_time);
  }

  TEST_CASE("construct_expiry") {
    auto expiry = time_from_string("2025-06-30 15:30:00");
    auto tif = TimeInForce(TimeInForce::Type::GTD, expiry);
    REQUIRE((tif.get_type() == TimeInForce::Type::GTD));
    REQUIRE(tif.get_expiry() == expiry);
  }

  TEST_CASE("stream") {
    auto tif = TimeInForce(TimeInForce::Type::MOC);
    REQUIRE(to_string(tif) == "MOC");
  }

  TEST_CASE("stream_expiry") {
    auto expiry = time_from_string("2025-06-30 08:45:00");
    auto tif = TimeInForce(TimeInForce::Type::GTX, expiry);
    REQUIRE(to_string(tif) == "(GTX " + to_string(expiry) + ")");
    test_round_trip_shuttle(tif);
  }
}
