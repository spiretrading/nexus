#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/AdministrationService/AccountIdentity.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::AdministrationService;

namespace {
  const auto& get_test_identity() {
    static auto identity = [&] {
      auto identity = AccountIdentity();
      identity.registration_time = time_from_string("2023-01-01 12:00:00");
      identity.last_login_time = time_from_string("2023-01-02 13:30:00");
      identity.first_name = "John";
      identity.last_name = "Doe";
      identity.email_address = "john.doe@example.com";
      identity.address_line_one = "123 Main St";
      identity.address_line_two = "Apt 4B";
      identity.address_line_three = "Building 5";
      identity.city = "Metropolis";
      identity.province = "State";
      identity.country = US;
      identity.photo_id.Append("hello world", std::strlen("hello world"));
      identity.user_notes = "Test user";
      return identity;
    }();
    return identity;
  }
}

TEST_SUITE("AccountIdentity") {
  TEST_CASE("stream") {
    auto out = std::ostringstream();
    out << get_test_identity();
    auto expected = "(\"2023-Jan-01 12:00:00\" \"2023-Jan-02 13:30:00\" "
      "\"John\" \"Doe\" \"john.doe@example.com\" \"123 Main St\" \"Apt 4B\" "
      "\"Building 5\" \"Metropolis\" \"State\" US \"Test user\")";
    CHECK(out.str() == expected);
  }

  TEST_CASE("shuttle") {
    Beam::Serialization::Tests::TestRoundTripShuttle(get_test_identity(),
      [] (const auto& received) {
        auto& expected = get_test_identity();
        REQUIRE(received.registration_time == expected.registration_time);
        REQUIRE(received.last_login_time == expected.last_login_time);
        REQUIRE(received.first_name == expected.first_name);
        REQUIRE(received.last_name == expected.last_name);
        REQUIRE(received.email_address == expected.email_address);
        REQUIRE(received.address_line_one == expected.address_line_one);
        REQUIRE(received.address_line_two == expected.address_line_two);
        REQUIRE(received.address_line_three == expected.address_line_three);
        REQUIRE(received.city == expected.city);
        REQUIRE(received.province == expected.province);
        REQUIRE(received.country == expected.country);
        REQUIRE(received.photo_id == expected.photo_id);
        REQUIRE(received.user_notes == expected.user_notes);
      });
  }
}
