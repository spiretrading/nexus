#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"

using namespace Nexus;
using namespace Nexus::OrderExecutionService;

namespace {
  template<typename T>
  struct SpecializedMakeDataStore {};

  template<>
  struct SpecializedMakeDataStore<LocalOrderExecutionDataStore> {
    auto operator ()() const {
      return LocalOrderExecutionDataStore();
    }
  };

  template<typename T>
  auto MakeDataStore() {
    return SpecializedMakeDataStore<T>()();
  };
}

TEST_CASE_TEMPLATE("query_order_records", T, LocalOrderExecutionDataStore) {
  auto dataStore = MakeDataStore<T>();
}
