#include <catch.hpp>
#include "spire/spire/qt_promise.hpp"
#include "spire/spire_tester/spire_tester.hpp"

using namespace Spire;

//! Returns a promise that signals the result only when all provided
//! promises have completed, or throws an exception if any provided
//! promise throws an exception.
/*
  \param promises The promises to be executed.
*/
template<typename T>
QtPromise<std::vector<T>> all(std::vector<QtPromise<T>> promises) {
  if(promises.empty()) {
    return std::move(QtPromise<std::vector<T>>());
  }
  // TODO: what to do if a promise throws an exception?
  // how to make this persist?
  auto completed = std::vector<T>();
  auto& promise = promises.front();
  return QtPromise<std::vector<T>>();
  //for(auto i = 1; i < promises.size(); ++i) {
  //  promise.then([&] (auto result) {
  //    completed.insert(completed.begin() + i, result.Get());
  //    return promises[i];
  //  });
  //}
  //return promise.then([&] (auto result) {
  //  completed.insert(completed.begin(), result.Get());
  //  return std::move(completed);
  //});
}

//TEST_CASE("test_empty_promise_vector", "[QtPromise]") {
//  auto promises = std::vector<QtPromise<std::vector<int>>>();
//  REQUIRE(wait(all(promises)) == std::vector<int>());
//}

TEST_CASE("test_single_promise", "[QtPromise]") {
  run_test([=] {
    auto promises = std::vector<QtPromise<std::vector<int>>>();
    promises.push_back(QtPromise([=] {
      return std::vector<int>({1, 2, 3});
    }));
    auto result = all(promises);
    //auto result = all(promises);
    REQUIRE(true == true);
  }, "test_single_promise");
}
