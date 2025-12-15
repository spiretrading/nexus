#include <doctest/doctest.h>
#include <algorithm>
#include <boost/optional/optional_io.hpp>
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("LocalQueryModel") {
  TEST_CASE("empty") {
    run_test([] {
      auto model = LocalQueryModel<int>();
      REQUIRE(model.parse("") == none);
      REQUIRE(model.parse("hello") == none);
      REQUIRE(wait(model.submit("hello")) == std::vector<int>());
    });
  }

  TEST_CASE("add") {
    run_test([] {
      auto model = LocalQueryModel<int>();
      model.add("hello", 123);
      model.add("hello world", 456);
      REQUIRE(model.parse("") == none);
      REQUIRE(model.parse("hello") == 123);
      REQUIRE(model.parse("hello world") == 456);
      REQUIRE(std::ranges::is_permutation(
        wait(model.submit("h")), std::vector{123, 456}));
      REQUIRE(std::ranges::is_permutation(
        wait(model.submit("hello")), std::vector{123, 456}));
      REQUIRE(wait(model.submit("hello world")) == std::vector{456});
    });
  }
}
