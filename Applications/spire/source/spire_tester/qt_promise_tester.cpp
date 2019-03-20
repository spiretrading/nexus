#include <catch.hpp>
#include "spire/spire/qt_promise.hpp"
#include "spire/spire_tester/spire_tester.hpp"

using namespace Spire;

TEST_CASE("test_chaining_promise_then", "[QtPromise]") {
  run_test([] {
    auto p = QtPromise(
      [] {
        return 123;
      }).then([] (auto result) {
        return QtPromise(
          [=] {
            return 2 * result.Get();
          }).then([] (auto result) {
            return 3 * result.Get();
          });
      }).then([] (auto result) {
        return 6 * result.Get();
      });
    auto r = wait(std::move(p));
    REQUIRE(r == 4428);
  }, "test_chaining_promise_then");
}

TEST_CASE("test_empty_promise", "[QtPromise]") {
  run_test([] {
    auto promises = std::vector<QtPromise<std::vector<int>>>();
    auto result = wait(std::move(all(std::move(promises))));
    REQUIRE(result == std::vector<std::vector<int>>());
  }, "test_empty_promise");
}

TEST_CASE("test_single_promise", "[QtPromise]") {
  run_test([] {
    auto promises = std::vector<QtPromise<int>>();
    promises.push_back(QtPromise([] {
      return 1;
    }));
    auto result = wait(std::move(all(std::move(promises))));
    REQUIRE(result == std::vector<int>{1});
  }, "test_single_promise");
}

TEST_CASE("test_multiple_promises", "[QtPromise]") {
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
    auto all_promise = std::move(all(std::move(promises)));
    auto result = wait(std::move(all_promise));
    REQUIRE(result == std::vector<int>{1, 2, 3, 4});
  }, "test_multiple_promises");
}
