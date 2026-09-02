#include <any>
#include <string>
#include <doctest/doctest.h>
#include "Spire/Spire/PropertyHub.hpp"

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

  TEST_CASE("adopt") {
    auto hub = PropertyHub();
    hub.get<int>("count")->set(123);
    hub.get<std::string>("name")->set("abc");
    auto adopter = PropertyHub();
    adopter.get<std::string>("name")->set("xyz");
    adopter.adopt(hub);
    REQUIRE(adopter.get<int>("count")->get() == 123);
    REQUIRE(adopter.get<std::string>("name")->get() == "xyz");
  }

  TEST_CASE("id") {
    auto hub = PropertyHub();
    REQUIRE(hub.get_id() != PropertyHub().get_id());
    auto id = hub.get_id();
    REQUIRE(PropertyHub(id).get_id() == id);
  }
}
