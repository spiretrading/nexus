#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TimeInForce.hpp"

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

  TEST_CASE("stream_type") {
    auto ss = std::ostringstream();
    ss << TimeInForce::Type::OPG;
    REQUIRE(ss.str() == "OPG");
  }

  TEST_CASE("stream") {
    auto tif = TimeInForce(TimeInForce::Type::MOC);
    auto ss = std::ostringstream();
    ss << tif;
    REQUIRE(ss.str() == "MOC");
  }

  TEST_CASE("steam_expiry") {
    auto expiry = time_from_string("2025-06-30 08:45:00");
    auto tif = TimeInForce(TimeInForce::Type::GTX, expiry);
    auto ss = std::ostringstream();
    ss << tif;
    auto output = ss.str();
    REQUIRE(output == "(GTX " + lexical_cast<std::string>(expiry) + ")");
  }
}
