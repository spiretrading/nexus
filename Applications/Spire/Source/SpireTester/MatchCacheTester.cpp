#include <doctest/doctest.h>
#include "Spire/KeyBindings/MatchCache.hpp"

using namespace Spire;

namespace {
  bool matches(const QString& value, const QString& query) {
    return value.toLower().startsWith(query.toLower());
  }
}

TEST_SUITE("MatchCache") {
  TEST_CASE("match_cache") {
    auto hit_cache = true;
    auto reset_hit_cache = [&] {
      hit_cache = true;
    };
    auto match_cache = MatchCache([&] (const QString& query) {
      hit_cache = false;
      return matches("hello", query);
    });
    REQUIRE(match_cache.matches("h"));
    REQUIRE(!hit_cache);
    reset_hit_cache();
    REQUIRE(match_cache.matches("hel"));
    REQUIRE(!hit_cache);
    reset_hit_cache();
    REQUIRE(match_cache.matches("hello"));
    REQUIRE(!hit_cache);
    reset_hit_cache();
    REQUIRE(match_cache.matches("h"));
    REQUIRE(hit_cache);
    reset_hit_cache();
    REQUIRE(match_cache.matches("hel"));
    REQUIRE(hit_cache);
    reset_hit_cache();
    REQUIRE(match_cache.matches("hello"));
    REQUIRE(hit_cache);
    reset_hit_cache();
    REQUIRE(!match_cache.matches("hello w"));
    REQUIRE(!hit_cache);
    reset_hit_cache();
    REQUIRE(!match_cache.matches("w"));
    REQUIRE(!hit_cache);
  }
}
