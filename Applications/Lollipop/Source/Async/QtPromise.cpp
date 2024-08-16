#include "Spire/Async/QtPromise.hpp"

using namespace Spire;

QtPromise<void> Spire::all(std::vector<QtPromise<void>> promises) {
  if(promises.empty()) {
    return QtPromise();
  }
  auto promise = std::move(promises.front());
  for(auto i = std::size_t(0); i < promises.size() - 1; ++i) {
    promise = promise.then([=, p = std::move(promises[i + 1])]
        (auto&& result) mutable {
      result.Get();
      return std::move(p);
    });
  }
  return promise;
}
