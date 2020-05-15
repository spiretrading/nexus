#include <doctest/doctest.h>
#include "Spire/Spire/QtFuture.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_SUITE("QtFuture") {
  TEST_CASE("immediate_value") {
    run_test([] {
      auto [f, p] = make_future<int>();
      f.resolve(12);
      REQUIRE(wait(std::move(p)) == 12);
    });
  }

  TEST_CASE("immediate_exception") {
    run_test([] {
      auto [f, p] = make_future<int>();
      f.resolve(std::make_exception_ptr(std::runtime_error("Failed future.")));
      REQUIRE_THROWS_WITH_AS(wait(std::move(p)), "Failed future.",
        std::runtime_error);
    });
  }
}
