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
      identity.m_registration_time = time_from_string("2023-01-01 12:00:00");
      identity.m_last_login_time = time_from_string("2023-01-02 13:30:00");
      identity.m_first_name = "John";
      identity.m_last_name = "Doe";
      identity.m_email_address = "john.doe@example.com";
      identity.m_address_line_one = "123 Main St";
      identity.m_address_line_two = "Apt 4B";
      identity.m_address_line_three = "Building 5";
      identity.m_city = "Metropolis";
      identity.m_province = "State";
      identity.m_country = US;
      identity.m_photo_id.Append("hello world", std::strlen("hello world"));
      identity.m_user_notes = "Test user";
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
        REQUIRE(received.m_registration_time == expected.m_registration_time);
        REQUIRE(received.m_last_login_time == expected.m_last_login_time);
        REQUIRE(received.m_first_name == expected.m_first_name);
        REQUIRE(received.m_last_name == expected.m_last_name);
        REQUIRE(received.m_email_address == expected.m_email_address);
        REQUIRE(received.m_address_line_one == expected.m_address_line_one);
        REQUIRE(received.m_address_line_two == expected.m_address_line_two);
        REQUIRE(received.m_address_line_three == expected.m_address_line_three);
        REQUIRE(received.m_city == expected.m_city);
        REQUIRE(received.m_province == expected.m_province);
        REQUIRE(received.m_country == expected.m_country);
        REQUIRE(received.m_photo_id == expected.m_photo_id);
        REQUIRE(received.m_user_notes == expected.m_user_notes);
      });
  }
}
