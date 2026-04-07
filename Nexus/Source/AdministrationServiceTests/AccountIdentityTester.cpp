#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AccountIdentity.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCountries;

TEST_SUITE("AccountIdentity") {
  TEST_CASE("stream") {
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
    append(identity.m_photo_id, "hello world");
    identity.m_user_notes = "Test user";
    auto expected = "(\"2023-Jan-01 12:00:00\" \"2023-Jan-02 13:30:00\" "
      "\"John\" \"Doe\" \"john.doe@example.com\" \"123 Main St\" \"Apt 4B\" "
      "\"Building 5\" \"Metropolis\" \"State\" US \"Test user\")";
    REQUIRE(to_string(identity) == expected);
    test_round_trip_shuttle(identity, [&] (const auto& received) {
      REQUIRE(received.m_registration_time == identity.m_registration_time);
      REQUIRE(received.m_last_login_time == identity.m_last_login_time);
      REQUIRE(received.m_first_name == identity.m_first_name);
      REQUIRE(received.m_last_name == identity.m_last_name);
      REQUIRE(received.m_email_address == identity.m_email_address);
      REQUIRE(received.m_address_line_one == identity.m_address_line_one);
      REQUIRE(received.m_address_line_two == identity.m_address_line_two);
      REQUIRE(received.m_address_line_three == identity.m_address_line_three);
      REQUIRE(received.m_city == identity.m_city);
      REQUIRE(received.m_province == identity.m_province);
      REQUIRE(received.m_country == identity.m_country);
      REQUIRE(received.m_photo_id == identity.m_photo_id);
      REQUIRE(received.m_user_notes == identity.m_user_notes);
    });
  }
}
