#include <doctest/doctest.h>
#include "Spire/Spire/LegacyPropertyHubMap.hpp"

using namespace Spire;

TEST_SUITE("LegacyPropertyHubMap") {
  TEST_CASE("acquire") {
    auto map = LegacyPropertyHubMap();
    auto a = map.acquire("a", "");
    auto b = map.acquire("b", "");
    REQUIRE(a != nullptr);
    REQUIRE(a != b);
    auto c = map.acquire("c", "d");
    auto d = map.acquire("d", "c");
    REQUIRE(c != a);
    REQUIRE(c == d);
    REQUIRE(map.acquire("e", "d") == c);
  }

  TEST_CASE("merge") {
    auto map = LegacyPropertyHubMap();
    auto merges = 0;
    auto source = std::shared_ptr<PropertyHub>();
    auto destination = std::shared_ptr<PropertyHub>();
    map.connect_merge_signal(
      [&] (const auto& merged_hub, const auto& surviving_hub) {
        ++merges;
        source = merged_hub;
        destination = surviving_hub;
      });
    auto a = map.acquire("a", "b");
    auto c = map.acquire("c", "d");
    REQUIRE(a != c);
    REQUIRE(merges == 0);
    auto b = map.acquire("b", "d");
    REQUIRE(merges == 1);
    REQUIRE(source == c);
    REQUIRE(destination == a);
    REQUIRE(b == a);
    REQUIRE(map.acquire("c", "") == a);
    REQUIRE(map.acquire("d", "") == a);
  }

  TEST_CASE("expired") {
    auto map = LegacyPropertyHubMap();
    auto id = map.acquire("a", "b")->get_id();
    auto hub = map.acquire("a", "b");
    REQUIRE(hub->get_id() != id);
    REQUIRE(map.acquire("b", "a") == hub);
  }
}
