#include <catch.hpp>
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam::Queries;
using namespace Nexus;
using namespace Spire;

namespace {
  std::vector<Candlestick> load(LocalChartModel model, int first, int last,
      const SnapshotLimit& limit) {
    return std::move(wait(model.load(ChartValue(first * Money::ONE),
      ChartValue(last * Money::ONE), limit)));
  }
}

TEST_CASE("test_storing_and_loading", "") {
  run_test([=] {
    auto model = LocalChartModel(ChartValue::Type::MONEY,
      ChartValue::Type::MONEY, {});

  }, "test_storing_and_loading");
}
