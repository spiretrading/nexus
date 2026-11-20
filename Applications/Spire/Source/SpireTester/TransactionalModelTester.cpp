#include <memory>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Spire/Spire/TransactionalModel.hpp"

using namespace Spire;

namespace {
  struct Fixture : TransactionalModel {
    int m_starts = 0;
    int m_ends = 0;
    bool m_throw_on_end = false;

    void start_transaction() override {
      ++m_starts;
    }

    void end_transaction() override {
      ++m_ends;
      if(m_throw_on_end) {
        throw std::runtime_error("End failed.");
      }
    }
  };
}

TEST_SUITE("TransactionalModel") {
  TEST_CASE("transaction_result") {
    auto model = Fixture();
    auto make_result = [&] {
      REQUIRE(model.m_starts == 1);
      REQUIRE(model.m_ends == 0);
      return std::make_unique<int>(7);
    };
    static_assert(std::same_as<decltype(model.transact(make_result)),
      std::unique_ptr<int>>);
    auto result = model.transact(make_result);
    REQUIRE(*result == 7);
    REQUIRE(model.m_ends == 1);
    auto& reference = model.transact([&] () -> int& { return *result; });
    REQUIRE(&reference == result.get());
    REQUIRE(model.m_starts == 2);
    REQUIRE(model.m_ends == 2);
  }

  TEST_CASE("transaction_exceptions") {
    auto model = Fixture();
    auto calls = 0;
    auto fail = [&] {
      ++calls;
      throw std::logic_error("Callback failed.");
    };
    SUBCASE("callback throws") {
      REQUIRE_THROWS_WITH_AS(
        model.transact(fail), "Callback failed.", std::logic_error);
    }
    SUBCASE("end throws") {
      model.m_throw_on_end = true;
      REQUIRE_THROWS_WITH_AS(
        model.transact([&] { ++calls; }), "End failed.", std::runtime_error);
    }
    SUBCASE("callback and end both throw") {
      model.m_throw_on_end = true;
      REQUIRE_THROWS_WITH_AS(
        model.transact(fail), "End failed.", std::runtime_error);
    }
    REQUIRE(calls == 1);
    REQUIRE(model.m_starts == 1);
    REQUIRE(model.m_ends == 1);
  }
}
