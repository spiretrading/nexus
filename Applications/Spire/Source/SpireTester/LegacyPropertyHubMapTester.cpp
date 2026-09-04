#include <memory>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/Spire/LegacyPropertyHubMap.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("LegacyPropertyHubMap") {
  TEST_CASE("acquire") {
    auto map = LegacyPropertyHubMap();
    REQUIRE(map.acquire("", "") != map.acquire("", ""));
    auto a = map.acquire("a", "");
    auto b = map.acquire("b", "");
    REQUIRE(a != nullptr);
    REQUIRE(a != b);
    auto c = map.acquire("c", "d");
    auto d = map.acquire("d", "c");
    REQUIRE(c != a);
    REQUIRE(c == d);
    REQUIRE(map.acquire("e", "d") == c);
    REQUIRE(map.acquire("", "d") == c);
  }

  TEST_CASE("merge") {
    auto merges = 0;
    auto source = std::shared_ptr<PropertyHub>();
    auto destination = std::shared_ptr<PropertyHub>();
    auto map = LegacyPropertyHubMap();
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
    c->get<int>("count")->set(7);
    auto b = map.acquire("b", "d");
    REQUIRE(merges == 1);
    REQUIRE(source == c);
    REQUIRE(destination == a);
    REQUIRE(b == a);
    REQUIRE(map.acquire("c", "") == a);
    REQUIRE(map.acquire("d", "") == a);
    REQUIRE(merges == 1);
    REQUIRE(a->get<int>("count")->get() == 7);
  }

  TEST_CASE("merge_ticker") {
    auto map = LegacyPropertyHubMap();
    auto abx = Ticker("ABX", Venue("TSX"));
    auto msft = Ticker("MSFT", Venue("XNAS"));
    auto a = map.acquire("a", "b");
    auto ticker = a->get<Ticker>(PropertyHub::TICKER_PROPERTY);
    auto c = map.acquire("c", "d");
    c->get<Ticker>(PropertyHub::TICKER_PROPERTY)->set(msft);
    map.acquire("b", "d");
    REQUIRE(a->get<Ticker>(PropertyHub::TICKER_PROPERTY) == ticker);
    REQUIRE(ticker->get() == msft);
    auto e = map.acquire("e", "f");
    e->get<Ticker>(PropertyHub::TICKER_PROPERTY)->set(abx);
    auto g = map.acquire("g", "h");
    g->get<Ticker>(PropertyHub::TICKER_PROPERTY)->set(msft);
    map.acquire("f", "h");
    REQUIRE(e->get<Ticker>(PropertyHub::TICKER_PROPERTY)->get() == abx);
    auto i = map.acquire("i", "j");
    auto k = map.acquire("k", "l");
    k->get<Ticker>(PropertyHub::TICKER_PROPERTY)->set(abx);
    map.acquire("j", "l");
    REQUIRE(i->get<Ticker>(PropertyHub::TICKER_PROPERTY) ==
      k->get<Ticker>(PropertyHub::TICKER_PROPERTY));
    REQUIRE(i->get<Ticker>(PropertyHub::TICKER_PROPERTY)->get() == abx);
  }

  TEST_CASE("expired") {
    auto map = LegacyPropertyHubMap();
    auto id = map.acquire("a", "b")->get_id();
    auto hub = map.acquire("a", "b");
    REQUIRE(hub->get_id() != id);
    REQUIRE(map.acquire("b", "a") == hub);
  }
}
