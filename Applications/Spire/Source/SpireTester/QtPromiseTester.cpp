#include <doctest/doctest.h>
#include "Spire/Async/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_SUITE("QtPromise") {
  TEST_CASE("immediate_value") {
    run_test([] {
      auto promise = QtPromise<int>([] { return 10; });
      auto result = wait(std::move(promise));
      REQUIRE(result == 10);
    });
  }

  TEST_CASE("promise_then") {
    run_test([] {
      auto promise = QtPromise([] {
        return 123;
      }).then([] (auto result) {
        return result + 1;
      }).then([] (auto result) {
        return result + 2;
      }).then([] (auto result) {
        return result + 3;
      }).then([] (auto result) {
        return result + 4;
      });
      auto result = wait(std::move(promise));
      REQUIRE(result == 133);
    });
  }

  TEST_CASE("chaining_promise_then") {
    run_test([] {
      auto promise = QtPromise([] {
        return 123;
      }).then([] (auto result) {
        return QtPromise([=] {
          return 2 * result.Get();
        }).then([] (auto result) {
          return 3 * result.Get();
        });
      }).then([] (auto result) {
        return 6 * result.Get();
      });
      auto result = wait(std::move(promise));
      REQUIRE(result == 4428);
    });
  }

  TEST_CASE("reassigned_chained_promise") {
    run_test([] {
      auto promise1 = QtPromise([] { return 5; });
      auto promise2 = promise1.then([] (auto&& result) {
        return 7 + result.Get();
      });
      auto result = wait(std::move(promise2));
      REQUIRE(result == 12);
    });
  }

  TEST_CASE("empty_promise") {
    run_test([] {
      auto promises = std::vector<QtPromise<std::vector<int>>>();
      auto result = wait(all(std::move(promises)));
      REQUIRE(result == std::vector<std::vector<int>>());
    });
  }

  TEST_CASE("single_promise") {
    run_test([] {
      auto promises = std::vector<QtPromise<int>>();
      promises.push_back(QtPromise([] {
        return 1;
      }));
      auto result = wait(all(std::move(promises)));
      REQUIRE(result == std::vector<int>{1});
    });
  }

  TEST_CASE("multiple_promises") {
    run_test([] {
      auto promises = std::vector<QtPromise<int>>();
      promises.push_back(QtPromise([] {
        return 1;
      }));
      promises.push_back(QtPromise([] {
        return 2;
      }));
      promises.push_back(QtPromise([] {
        return 3;
      }));
      promises.push_back(QtPromise([] {
        return 4;
      }));
      auto all_promise = all(std::move(promises));
      auto result = wait(std::move(all_promise));
      REQUIRE(result == std::vector<int>{1, 2, 3, 4});
    });
  }

  TEST_CASE("move_only_type") {
    run_test([] {
      auto promises = std::vector<QtPromise<std::unique_ptr<int>>>();
      promises.push_back(QtPromise([] {
        return std::make_unique<int>(1);
      }));
      promises.push_back(QtPromise([] {
        return std::make_unique<int>(2);
      }));
      promises.push_back(QtPromise([] {
        return std::make_unique<int>(3);
      }));
      promises.push_back(QtPromise([] {
        return std::make_unique<int>(4);
      }));
      auto expected_result = std::vector<std::unique_ptr<int>>();
      expected_result.push_back(std::make_unique<int>(1));
      expected_result.push_back(std::make_unique<int>(2));
      expected_result.push_back(std::make_unique<int>(3));
      expected_result.push_back(std::make_unique<int>(4));
      auto all_promise = all(std::move(promises));
      auto result = wait(std::move(all_promise));
      REQUIRE(std::equal(result.begin(), result.end(), expected_result.begin(),
        expected_result.end(), [](const auto& lhs, const auto& rhs) {
          return *lhs == *rhs;
        }));
    });
  }

  TEST_CASE("void_all") {
    run_test([] {
      auto value = 0;
      auto promises = std::vector<QtPromise<void>>();
      promises.push_back(QtPromise([&] {
        ++value;
      }));
      promises.push_back(QtPromise([&] {
        ++value;
      }));
      promises.push_back(QtPromise([&] {
        ++value;
      }));
      promises.push_back(QtPromise([&] {
        ++value;
      }));
      auto all_promise = all(std::move(promises));
      wait(std::move(all_promise));
      REQUIRE(value == 4);
    });
  }

  TEST_CASE("void_then") {
    run_test([] {
      auto x = 0;
      auto p = QtPromise([&] {
        x += 5;
      }).then([&] (Beam::Expect<void>) {
        x += 10;
      });
      wait(std::move(p));
      REQUIRE(x == 15);
    });
  }
}
