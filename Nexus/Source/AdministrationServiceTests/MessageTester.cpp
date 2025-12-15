#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/Message.hpp"

using namespace Nexus;
using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("message") {
  TEST_CASE("body_stream") {
    auto body = Message::Body();
    body.m_content_type = "text/plain";
    body.m_message = "hello";
    REQUIRE(to_string(body) == "(text/plain hello)");
  }

  TEST_CASE("make_plain_text") {
    auto body = Message::Body::make_plain_text("sample");
    REQUIRE(body.m_content_type == "text/plain");
    REQUIRE(body.m_message == "sample");
  }

  TEST_CASE("shuttle") {
    auto bodies = std::vector<Message::Body>();
    bodies.push_back(Message::Body::make_plain_text("first"));
    bodies.push_back(Message::Body::make_plain_text("second"));
    auto message = Message(5, DirectoryEntry::make_account(10, "user"),
      time_from_string("2024-07-04 12:00:00"), bodies);
    test_round_trip_shuttle(message, [&] (const auto& received) {
      REQUIRE(received.get_id() == message.get_id());
      REQUIRE(received.get_account() == message.get_account());
      REQUIRE(received.get_timestamp() == message.get_timestamp());
      REQUIRE(received.get_bodies() == message.get_bodies());
      REQUIRE(received.get_body() == message.get_body());
    });
  }
}
