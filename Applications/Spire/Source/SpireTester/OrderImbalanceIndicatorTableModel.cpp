#include <doctest/doctest.h>
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"

using namespace Beam::TimeService;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("OrderImbalanceIndicatorTableModel") {
  TEST_CASE("set_interval") {
    auto model = std::make_shared<OrderImbalanceIndicatorTableModel>(
      std::make_shared<LocalOrderImbalanceIndicatorModel>(),
      std::make_shared<FixedTimeClient>());
  }

  TEST_CASE("set_offset") {
  
  }

  TEST_CASE("published_imbalances") {
  
  }
}
