#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderWrapperReactor.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

TEST_SUITE("OrderWrapperReactor") {
  TEST_CASE("single_order_evaluation") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.push(true);
      });
    auto security = Security("TST", TSX);
    auto fields =
      make_limit_order_fields(security, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto order = std::make_shared<PrimitiveOrder>(
      OrderInfo(fields, 1, false, time_from_string("2024-07-21 10:00:00.000")));
    auto reactor = OrderWrapperReactor(order);
  }
}
