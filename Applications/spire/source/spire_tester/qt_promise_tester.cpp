#include <catch.hpp>
#include "spire/spire/qt_promise.hpp"
#include "spire/spire_tester/spire_tester.hpp"

using namespace Spire;

//QT_TEST_CASE("test_single_promise", "[QtPromise]") {
//}

//! Returns a promise that signals the result only when all provided
//! promises have completed, or throws an exception if any provided
//! promise throws an exception.
/*
  \param promises The promises to be executed.
*/
template<typename T>
QtPromise<std::vector<T>> all(std::vector<QtPromise<T>> promises) {
  if(promises.empty()) {
    return {};
  }
  // TODO: what to do if a promise throws an exception?
  auto completed_promises = std::make_shared<std::vector<T>>();
  auto promise = std::move(promises.front());
  //for(auto i = 1; i < promises.size(); ++i) {
  //  promise = promise.then([&] (auto result) {
  //    completed_promises->insert(completed_promises->begin() + i,
  //      result.Get());
  //    return std::move(promises[i]);
  //  });
  //}
  return promise.then([=] (auto result) {
    completed_promises->insert(completed_promises->begin(), result.Get());
    return QtPromise(
      [=] {
        return *completed_promises;
      });
  });
  //return promise.then([&] (auto result) {
  //  completed_promises->insert(completed_promises->begin(), result.Get());
  //  return std::move(*completed_promises);
  //});
}

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

TEST_CASE("test_empty_promise_vector", "[QtPromise]") {
  auto promises = std::vector<QtPromise<std::vector<int>>>();
  REQUIRE(wait(all(std::move(promises))) == std::vector<std::vector<int>>());
}

TEST_CASE("test_single_promise", "[QtPromise]") {
  run_test([=] {
    auto promises = std::vector<QtPromise<int>>();
    promises.push_back(QtPromise([] {
      return 1;
    }));
    auto all_promise = all(std::move(promises));
    auto result = wait(std::move(all_promise));
  }, "test_single_promise");
}
