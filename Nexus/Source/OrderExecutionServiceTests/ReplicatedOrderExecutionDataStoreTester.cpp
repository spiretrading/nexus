#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/ReplicatedOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionDataStoreTestSuite.hpp"

using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      auto replicated_data_stores = std::vector<OrderExecutionDataStore>();
      replicated_data_stores.push_back(OrderExecutionDataStore(
        std::in_place_type<LocalOrderExecutionDataStore>));
      return ReplicatedOrderExecutionDataStore(OrderExecutionDataStore(
        std::in_place_type<LocalOrderExecutionDataStore>),
        std::move(replicated_data_stores));
    }
  };
}

TEST_SUITE("ReplicatedOrderExecutionDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(OrderExecutionDataStoreTestSuite, Builder);
}
