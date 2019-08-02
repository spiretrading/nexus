#include <catch.hpp>
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam::Queries;
using namespace Nexus;
using namespace Spire;

namespace {
  std::vector<Candlestick> load(LocalChartModel* model, int first, int last,
      const SnapshotLimit& limit) {
    return std::move(wait(model->load(ChartValue(first * Money::ONE),
      ChartValue(last * Money::ONE), limit)));
  }

  std::vector<Candlestick> generate_range(int first, int last) {
    auto candlesticks = std::vector<Candlestick>();
    for(auto i = first; i < last; ++i) {
      candlesticks.push_back({ChartValue(i), ChartValue(i + 1), ChartValue(2),
        ChartValue(6), ChartValue(8), ChartValue(0)});
    }
    return candlesticks;
  }

  Candlestick make(int start, int end) {
    return {ChartValue(start), ChartValue(end),
      ChartValue(2), ChartValue(6), ChartValue(8), ChartValue(0)};
  }
}

TEST_CASE("test_storing_and_loading", "[LocalChartModel]") {
  run_test([=] {
    auto model = LocalChartModel(ChartValue::Type::MONEY,
      ChartValue::Type::MONEY, {});
    auto load1 = load(&model, 0, 100, SnapshotLimit::Unlimited());
    REQUIRE(load1.size() == 0);
    model.store(generate_range(10, 20));
    auto load2 = load(&model, 0, 25, SnapshotLimit::Unlimited());
    REQUIRE(load2.size() == 10);
    REQUIRE(load2.front().GetStart() == ChartValue(10));
    REQUIRE(load2.back().GetEnd() == ChartValue(20));
    model.store(generate_range(30, 40));
    auto load3 = load(&model, 25, 50, SnapshotLimit::Unlimited());
    REQUIRE(load3.size() == 10);
    REQUIRE(load3.front().GetStart() == ChartValue(30));
    REQUIRE(load3.back().GetEnd() == ChartValue(40));
    auto load4 = load(&model, 0, 100, SnapshotLimit::Unlimited());
    REQUIRE(load4.size() == 20);
    REQUIRE(load4.front().GetStart() == ChartValue(10));
    REQUIRE(load4.back().GetEnd() == ChartValue(40));
  }, "test_storing_and_loading");
}

TEST_CASE("test_merging_data", "[LocalChartModel]") {
  run_test([=] {
    auto model = LocalChartModel(ChartValue::Type::MONEY,
      ChartValue::Type::MONEY, {});
    model.store({make(0, 1), make(2, 3), make(4, 5), make(6, 7), make(8, 9)});
    auto load1 = load(&model, 0, 10, SnapshotLimit::Unlimited());
    REQUIRE(load1.size() == 5);
    model.store({make(1, 2), make(3, 4), make(5, 6), make(7, 8), make(9, 10)});
    auto load2 = load(&model, 0, 10, SnapshotLimit::Unlimited());
    REQUIRE(load2.size() == 10);
  }, "test_merging_data");
}
