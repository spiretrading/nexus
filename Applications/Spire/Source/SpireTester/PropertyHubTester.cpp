#include <any>
#include <string>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Spire/Spire/PropertyHub.hpp"
#include "Spire/Spire/PropertyHubSerialization.hpp"

using namespace Beam::Tests;
using namespace Spire;

TEST_SUITE("PropertyHub") {
  TEST_CASE("get") {
    auto hub = PropertyHub();
    auto count = hub.get<int>("count");
    REQUIRE(count->get() == 0);
    count->set(123);
    REQUIRE(hub.get<int>("count") == count);
    REQUIRE_THROWS_AS(hub.get<double>("count"), std::bad_any_cast);
  }

  TEST_CASE("find") {
    auto hub = PropertyHub();
    REQUIRE(hub.find("count") == nullptr);
    auto count = hub.get<int>("count");
    count->set(123);
    auto property = hub.find("count");
    REQUIRE(property != nullptr);
    REQUIRE(any_cast<int>(property->get()) == 123);
  }

  TEST_CASE("remove") {
    auto hub = PropertyHub();
    hub.get<int>("count")->set(123);
    hub.remove("count");
    REQUIRE(hub.find("count") == nullptr);
    REQUIRE(hub.get<int>("count")->get() == 0);
  }

  TEST_CASE("id") {
    auto hub = PropertyHub();
    REQUIRE(hub.get_id() != PropertyHub().get_id());
  }

  TEST_CASE("shuttle") {
    auto hub = PropertyHub();
    hub.get<int>("count")->set(123);
    hub.get<std::string>("name")->set("abc");
    test_polymorphic_round_trip_shuttle(hub,
      [] (auto registry) {
        RegisterPropertyHubTypes(out(registry));
      },
      [&] (auto&& received) {
        REQUIRE(received.get_id() == hub.get_id());
        REQUIRE(received.get<int>("count")->get() == 123);
        REQUIRE(received.get<std::string>("name")->get() == "abc");
      });
  }
}
