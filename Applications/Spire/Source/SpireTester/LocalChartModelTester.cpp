#include <catch.hpp>
#include "Spire/Charting/CachedChartModel.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/SpireTester/TestChartModel.hpp"

using namespace Beam::Queries;
using namespace Nexus;
using namespace Spire;

namespace {
  auto create_model() {
    auto local_model = std::make_shared<LocalChartModel>(
      ChartValue::Type::MONEY, ChartValue::Type::MONEY,
      [=] {
        auto candlesticks = std::vector<Candlestick>();
        for(auto i = 0; i < 100; ++i) {
          candlesticks.push_back({ChartValue(i * Money::ONE),
            ChartValue((i + 1) * Money::ONE)});
        }
        return candlesticks;
      }());
    return local_model;
  }

  std::vector<Candlestick> load(int first, int last,
      const SnapshotLimit& limit) {
    auto model = create_model();
    return std::move(wait(model->load(ChartValue(first * Money::ONE),
      ChartValue(last * Money::ONE), limit)));
  }
}

TEST_CASE("test_unlimited_loads", "[LocalChartModel]") {
  run_test([=] {
    auto load1 = load(0, 100, SnapshotLimit::Unlimited());
    REQUIRE(load1.size() == 100);
    REQUIRE(load1.front().GetStart() == ChartValue(0));
    REQUIRE(load1.back().GetEnd() == ChartValue(100));
    auto load2 = load(25, 75, SnapshotLimit::Unlimited());
    REQUIRE(load2.size() == 52);
    REQUIRE(load2.front().GetStart() == ChartValue(24));
    REQUIRE(load2.back().GetEnd() == ChartValue(76));
    auto load3 = load(-10, 25, SnapshotLimit::Unlimited());
    REQUIRE(load3.size() == 26);
    REQUIRE(load3.front().GetStart() == ChartValue(0));
    REQUIRE(load3.back().GetEnd() == ChartValue(26));
    auto load4 = load(75, 110, SnapshotLimit::Unlimited());
    REQUIRE(load4.size() == 26);
    REQUIRE(load4.front().GetStart() == ChartValue(74));
    REQUIRE(load4.back().GetEnd() == ChartValue(100));
  }, "test_unlimited_loads");
}

TEST_CASE("test_loads_from_head", "[LocalChartModel]") {
  run_test([=] {
    auto load1 = load(0, 100, SnapshotLimit::FromHead(25));
    REQUIRE(load1.size() == 25);
    REQUIRE(load1.front().GetStart() == ChartValue(0));
    REQUIRE(load1.back().GetEnd() == ChartValue(25));
    auto load2 = load(50, 100, SnapshotLimit::FromHead(25));
    REQUIRE(load2.size() == 25);
    REQUIRE(load2.front().GetStart() == ChartValue(49));
    REQUIRE(load2.back().GetEnd() == ChartValue(74));
    auto load3 = load(-10, 25, SnapshotLimit::FromHead(10));
    REQUIRE(load3.size() == 10);
    REQUIRE(load3.front().GetStart() == ChartValue(0));
    REQUIRE(load3.back().GetEnd() == ChartValue(10));
    auto load4 = load(95, 110, SnapshotLimit::FromHead(10));
    REQUIRE(load4.size() == 6);
    REQUIRE(load4.front().GetStart() == ChartValue(94));
    REQUIRE(load4.back().GetEnd() == ChartValue(100));
  }, "test_loads_from_head");
}

TEST_CASE("test_loads_from_tail", "[LocalChartModel]") {
  run_test([=] {
    auto load1 = load(0, 100, SnapshotLimit::FromTail(25));
    REQUIRE(load1.size() == 25);
    REQUIRE(load1.front().GetStart() == ChartValue(75));
    REQUIRE(load1.back().GetEnd() == ChartValue(100));
    auto load2 = load(50, 100, SnapshotLimit::FromTail(25));
    REQUIRE(load2.size() == 25);
    REQUIRE(load2.front().GetStart() == ChartValue(75));
    REQUIRE(load2.back().GetEnd() == ChartValue(100));
    auto load3 = load(-10, 25, SnapshotLimit::FromTail(10));
    REQUIRE(load3.size() == 10);
    REQUIRE(load3.front().GetStart() == ChartValue(16));
    REQUIRE(load3.back().GetEnd() == ChartValue(26));
    auto load4 = load(95, 110, SnapshotLimit::FromTail(10));
    REQUIRE(load4.size() == 6);
    REQUIRE(load4.front().GetStart() == ChartValue(94));
    REQUIRE(load4.back().GetEnd() == ChartValue(100));
  }, "test_loads_from_tail");
}
