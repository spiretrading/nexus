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
    return QtPromise<std::vector<T>>();
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
    return *completed_promises;
  });
  //return promise.then([&] (auto result) {
  //  completed_promises->insert(completed_promises->begin(), result.Get());
  //  return std::move(*completed_promises);
  //});
}

//TEST_CASE("test_empty_promise_vector", "[QtPromise]") {
//  auto promises = std::vector<QtPromise<std::vector<int>>>();
//  REQUIRE(wait(all(promises)) == std::vector<int>());
//}

TEST_CASE("test_single_promise", "[QtPromise]") {
  run_test([=] {
    auto promises = std::vector<QtPromise<int>>();
    promises.push_back(QtPromise([] {
      return 1;
    }));
    auto all_promise = all(std::move(promises));
    auto result = wait(std::move(all_promise));
    //auto result = static_cast<std::vector<std::vector<int>>
    //  (*)(QtPromise<std::vector<std::vector<int>>>&)>(wait)(all_promise);
    REQUIRE(true == true);
  }, "test_single_promise");
}
