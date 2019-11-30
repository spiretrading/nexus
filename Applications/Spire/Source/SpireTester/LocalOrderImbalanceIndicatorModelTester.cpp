#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const std::string& symbol, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, 0), Side::BID, 100,
      Money(Money::ONE), timestamp);
  }

  const auto A = make_imbalance("A", from_time_t(100));
  const auto B = make_imbalance("B", from_time_t(200));
  const auto C = make_imbalance("C", from_time_t(300));
  const auto D = make_imbalance("D", from_time_t(400));
  const auto E = make_imbalance("E", from_time_t(500));
}
