#include <doctest/doctest.h>
#include "Spire/Async/QtFuture.hpp"
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

  TEST_CASE("void_future_immediate_resolution") {
    run_test([] {
      auto [f, p] = make_future<void>();
      f.resolve();
      wait(std::move(p));
    });
  }

  TEST_CASE("void_future_immediate_exception") {
    run_test([] {
      auto [f, p] = make_future<void>();
      f.resolve(std::make_exception_ptr(std::runtime_error("Failed future.")));
      REQUIRE_THROWS_WITH_AS(wait(std::move(p)), "Failed future.",
        std::runtime_error);
    });
  }
}
