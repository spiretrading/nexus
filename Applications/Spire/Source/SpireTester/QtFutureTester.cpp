#include <doctest/doctest.h>
#include "Spire/Async/QtFuture.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Spire;

TEST_SUITE("QtFuture") {
  UI_TEST_CASE("immediate_value") {
    auto [f, p] = make_future<int>();
    f.resolve(12);
    REQUIRE(wait(std::move(p)) == 12);
  }

  UI_TEST_CASE("immediate_exception") {
    auto [f, p] = make_future<int>();
    f.resolve(std::make_exception_ptr(std::runtime_error("Failed future.")));
    REQUIRE_THROWS_WITH_AS(
      wait(std::move(p)), "Failed future.", std::runtime_error);
  }

  UI_TEST_CASE("void_future_immediate_resolution") {
    auto [f, p] = make_future<void>();
    f.resolve();
    wait(std::move(p));
  }

  UI_TEST_CASE("void_future_immediate_exception") {
    auto [f, p] = make_future<void>();
    f.resolve(std::make_exception_ptr(std::runtime_error("Failed future.")));
    REQUIRE_THROWS_WITH_AS(
      wait(std::move(p)), "Failed future.", std::runtime_error);
  }

  UI_TEST_CASE("unresolved_future") {
    auto promise = [] {
      auto [f, p] = make_future<int>();
      return std::move(p);
    }();
    REQUIRE_THROWS_WITH_AS(
      wait(std::move(promise)), "Broken future.", std::runtime_error);
  }

  UI_TEST_CASE("void_future_unresolved") {
    auto promise = [] {
      auto [f, p] = make_future<void>();
      return std::move(p);
    }();
    REQUIRE_THROWS_WITH_AS(
      wait(std::move(promise)), "Broken future.", std::runtime_error);
  }

  UI_TEST_CASE("reassigned_future") {
    auto [f1, p1] = make_future<int>();
    auto [f2, p2] = make_future<int>();
    f1 = std::move(f2);
    REQUIRE_THROWS_WITH_AS(
      wait(std::move(p1)), "Broken future.", std::runtime_error);
  }
}
