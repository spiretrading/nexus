#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_SUITE("AnyQueryModel") {
  TEST_CASE("wrapper") {
    run_test([] {
      auto source = std::make_shared<LocalQueryModel<int>>();
      source->add("hello", 123);
      source->add("world", 456);
      auto& any_model = static_cast<AnyQueryModel&>(*source);
      auto p1 = any_model.parse("hello");
      REQUIRE(p1.has_value());
      REQUIRE(std::any_cast<int>(p1) == 123);
      auto p2 = any_model.parse("world");
      REQUIRE(p2.has_value());
      REQUIRE(std::any_cast<int>(p2) == 456);
      auto p3 = any_model.parse("goodbye");
      REQUIRE(!p3.has_value());
      auto s1 = wait(any_model.submit("hell"));
      REQUIRE(s1.size() == 1);
      REQUIRE(std::any_cast<int>(s1[0]) == 123);
      auto s2 = wait(any_model.submit("wo"));
      REQUIRE(s2.size() == 1);
      REQUIRE(std::any_cast<int>(s2[0]) == 456);
      auto s3 = wait(any_model.submit("goodb"));
      REQUIRE(s3.empty());
    });
  }
}
