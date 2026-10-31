#include <chrono>
#include <optional>
#include <doctest/doctest.h>
#include "Spire/Async/QtFuture.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Spire;

namespace {
  template<typename T>
  std::optional<Expect<T>> try_wait(QtPromise<T> promise) {
    auto result = std::optional<Expect<T>>();
    auto continuation = promise.then([&] (auto&& value) {
      result.emplace(std::forward<decltype(value)>(value));
    });
    auto expiry = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while(!result && std::chrono::steady_clock::now() < expiry) {
      QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    return result;
  }
}

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
    auto result = try_wait([] {
      auto [f, p] = make_future<int>();
      return std::move(p);
    }());
    REQUIRE(result);
    REQUIRE_THROWS_WITH_AS(result->get(), "Broken future.", std::runtime_error);
  }

  UI_TEST_CASE("void_future_unresolved") {
    auto result = try_wait([] {
      auto [f, p] = make_future<void>();
      return std::move(p);
    }());
    REQUIRE(result);
    REQUIRE_THROWS_WITH_AS(result->get(), "Broken future.", std::runtime_error);
  }

  UI_TEST_CASE("reassigned_future") {
    auto [f1, p1] = make_future<int>();
    auto [f2, p2] = make_future<int>();
    f1 = std::move(f2);
    auto result = try_wait(std::move(p1));
    REQUIRE(result);
    REQUIRE_THROWS_WITH_AS(result->get(), "Broken future.", std::runtime_error);
  }
}
