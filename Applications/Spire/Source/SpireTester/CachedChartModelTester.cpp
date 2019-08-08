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
        for(auto i = 0; i < 101; ++i) {
          candlesticks.push_back({ChartValue(i * Money::ONE),
            ChartValue((i + 1) * Money::ONE)});
        }
        return candlesticks;
      }());
    return local_model;
  }

  std::vector<Candlestick> load(ChartModel* model, int first, int last,
      const SnapshotLimit& limit) {
    return std::move(wait(model->load(ChartValue(first * Money::ONE),
      ChartValue(last * Money::ONE), limit)));
  }

  Candlestick make(int start, int end) {
    return {ChartValue(start), ChartValue(end)};
  }

  auto create_trivial_coincident_model() {
    auto model = std::make_shared<LocalChartModel>(
      ChartValue::Type::MONEY, ChartValue::Type::MONEY,
      [=] {
        auto candlesticks = std::vector<Candlestick>();
        for(auto i = 0; i < 101; ++i) {
          auto candlestick = Candlestick(ChartValue(i * Money::ONE),
            ChartValue((i + 1) * Money::ONE));
          candlesticks.insert(candlesticks.end(), 5, candlestick);
        }
        return candlesticks;
      }());
    return model;
  }

  auto create_nontrivial_coincident_model() {
    auto model = std::make_shared<LocalChartModel>(
      ChartValue::Type::MONEY, ChartValue::Type::MONEY,
      [=] {
        auto candlesticks = std::vector<Candlestick>();
        candlesticks.push_back(make(1, 40));
        candlesticks.push_back(make(10, 40));
        candlesticks.push_back(make(20, 40));
        candlesticks.push_back(make(30, 40));
        candlesticks.push_back(make(39, 40));
        candlesticks.push_back(make(40, 41));
        candlesticks.push_back(make(41, 42));
        candlesticks.push_back(make(42, 43));
        candlesticks.push_back(make(43, 44));
        candlesticks.push_back(make(44, 45));
        candlesticks.push_back(make(45, 46));
        candlesticks.push_back(make(45, 50));
        candlesticks.push_back(make(45, 60));
        candlesticks.push_back(make(45, 70));
        candlesticks.push_back(make(45, 80));
        return candlesticks;
      }());
    return model;
  }
}

TEST_CASE("test_right_no_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 10, 20, SnapshotLimit::Unlimited());
    auto cached_sticks = load(&cache, 10, 20, SnapshotLimit::Unlimited());
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 30, 40, SnapshotLimit::Unlimited());
    auto test_sticks = load(&cache, 30, 40, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
  }, "test_right_no_overlap");
}

TEST_CASE("test_left_no_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 20, 30, SnapshotLimit::Unlimited());
    auto cached_sticks = load(&cache, 20, 30, SnapshotLimit::Unlimited());
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 10, 18, SnapshotLimit::Unlimited());
    auto test_sticks = load(&cache, 10, 18, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
  }, "test_left_no_overlap");
}

TEST_CASE("test_right_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 25, 50, SnapshotLimit::Unlimited());
    auto cached_sticks = load(&cache, 25, 50, SnapshotLimit::Unlimited());
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
    auto test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
  }, "test_right_overlap");
}

TEST_CASE("test_left_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 25, 50, SnapshotLimit::Unlimited());
    auto cached_sticks = load(&cache, 25, 50, SnapshotLimit::Unlimited());
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 10, 30, SnapshotLimit::Unlimited());
    auto test_sticks = load(&cache, 10, 30, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
  }, "test_left_overlap");
}

TEST_CASE("test_subset", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 25, 75, SnapshotLimit::Unlimited());
    auto cached_sticks = load(&cache, 25, 75, SnapshotLimit::Unlimited());
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
    auto test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
  }, "test_subset");
}

TEST_CASE("test_single_superset", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
    auto cached_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 20, 80, SnapshotLimit::Unlimited());
    auto test_sticks = load(&cache, 20, 80, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
  }, "test_single_superset");
}

TEST_CASE("test_multiple_supersets", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
    auto cached_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 20, 80, SnapshotLimit::Unlimited());
    auto test_sticks = load(&cache, 20, 80, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 10, 90, SnapshotLimit::Unlimited());
    test_sticks = load(&cache, 10, 90, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
  }, "test_multiple_supersets");
}

TEST_CASE("test_multiple_subsets_and_supersets", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 40, 50, SnapshotLimit::Unlimited());
    auto cached_sticks = load(&cache, 40, 50, SnapshotLimit::Unlimited());
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 60, 70, SnapshotLimit::Unlimited());
    auto test_sticks = load(&cache, 60, 70, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 80, 90, SnapshotLimit::Unlimited());
    test_sticks = load(&cache, 80, 90, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 45, 72, SnapshotLimit::Unlimited());
    test_sticks = load(&cache, 45, 72, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 35, 95, SnapshotLimit::Unlimited());
    test_sticks = load(&cache, 35, 95, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
    test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
    REQUIRE(test_sticks == model_sticks);
  }, "test_multiple_subsets_and_supersets");
}

TEST_CASE("test_multiple_cache_hits", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto test_model = TestChartModel(model->get_x_axis_type(),
      model->get_y_axis_type());
    auto cache = CachedChartModel(test_model);
    auto pre_load1 = cache.load(ChartValue(40 * Money::ONE),
      ChartValue(50 * Money::ONE), SnapshotLimit::Unlimited());
    wait(test_model.pop_load())->set_result(wait(
      model->load(ChartValue(40 * Money::ONE), ChartValue(50 * Money::ONE),
      SnapshotLimit::Unlimited())));
    wait(std::move(pre_load1));
    auto pre_load2 = cache.load(ChartValue(60 * Money::ONE),
      ChartValue(70 * Money::ONE), SnapshotLimit::Unlimited());
    wait(test_model.pop_load())->set_result(wait(
      model->load(ChartValue(60 * Money::ONE), ChartValue(70 * Money::ONE),
      SnapshotLimit::Unlimited())));
    wait(std::move(pre_load2));
    cache.load(ChartValue(45 * Money::ONE),
      ChartValue(75 * Money::ONE), SnapshotLimit::Unlimited());
    auto cache_load1 = wait(test_model.pop_load());
    REQUIRE(cache_load1->get_first() == ChartValue(50 * Money::ONE));
    REQUIRE(cache_load1->get_last() == ChartValue(60 * Money::ONE));
    cache_load1->set_result({});
    auto cache_load2 = wait(test_model.pop_load());
    REQUIRE(cache_load2->get_first() == ChartValue(70 * Money::ONE));
    REQUIRE(cache_load2->get_last() == ChartValue(75 * Money::ONE));
    cache_load2->set_result({});
  }, "test_multiple_cache_hits");
}

TEST_CASE("test_cached_model_loads_from_head", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    model->load(ChartValue(0), ChartValue(100), SnapshotLimit::Unlimited());
    auto load1 = load(&cache, 0, 100, SnapshotLimit::FromHead(25));
    REQUIRE(load1.size() == 25);
    REQUIRE(load1.front().GetStart() == ChartValue(0));
    REQUIRE(load1.back().GetEnd() == ChartValue(25));
    auto load2 = load(&cache, 50, 100, SnapshotLimit::FromHead(25));
    REQUIRE(load2.size() == 25);
    REQUIRE(load2.front().GetStart() == ChartValue(49));
    REQUIRE(load2.back().GetEnd() == ChartValue(74));
    auto load3 = load(&cache, -10, 25, SnapshotLimit::FromHead(10));
    REQUIRE(load3.size() == 10);
    REQUIRE(load3.front().GetStart() == ChartValue(0));
    REQUIRE(load3.back().GetEnd() == ChartValue(10));
    auto load4 = load(&cache, 95, 110, SnapshotLimit::FromHead(10));
    REQUIRE(load4.size() == 7);
    REQUIRE(load4.front().GetStart() == ChartValue(94));
    REQUIRE(load4.back().GetEnd() == ChartValue(101));
  }, "test_cached_model_loads_from_head");
}

TEST_CASE("test_cache_model_loads_from_tail", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    model->load(ChartValue(0), ChartValue(100), SnapshotLimit::Unlimited());
    auto load1 = load(&cache, 0, 100, SnapshotLimit::FromTail(25));
    REQUIRE(load1.size() == 25);
    REQUIRE(load1.front().GetStart() == ChartValue(76));
    REQUIRE(load1.back().GetEnd() == ChartValue(101));
    auto load2 = load(&cache, 50, 100, SnapshotLimit::FromTail(25));
    REQUIRE(load2.size() == 25);
    REQUIRE(load2.front().GetStart() == ChartValue(76));
    REQUIRE(load2.back().GetEnd() == ChartValue(101));
    auto load3 = load(&cache, -10, 25, SnapshotLimit::FromTail(10));
    REQUIRE(load3.size() == 10);
    REQUIRE(load3.front().GetStart() == ChartValue(16));
    REQUIRE(load3.back().GetEnd() == ChartValue(26));
    auto load4 = load(&cache, 95, 110, SnapshotLimit::FromTail(10));
    REQUIRE(load4.size() == 7);
    REQUIRE(load4.front().GetStart() == ChartValue(94));
    REQUIRE(load4.back().GetEnd() == ChartValue(101));
  }, "test_cache_model_loads_from_tail");
}

TEST_CASE("test_cached_coincidental_values_with_limits", "[LocalChartModel]") {
  run_test([=] {
    auto model = create_nontrivial_coincident_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 40, 40, SnapshotLimit::FromHead(3));
    auto cache_sticks = load(&cache, 40, 40, SnapshotLimit::FromHead(3));
    REQUIRE(cache_sticks == model_sticks);
    model_sticks = load(model.get(), 40, 40, SnapshotLimit::FromHead(5));
    cache_sticks = load(&cache, 40, 40, SnapshotLimit::FromHead(5));
    REQUIRE(cache_sticks == model_sticks);
    model_sticks = load(model.get(), 45, 45, SnapshotLimit::FromTail(3));
    cache_sticks = load(&cache, 45, 45, SnapshotLimit::FromTail(5));
    REQUIRE(cache_sticks == model_sticks);
    model_sticks = load(model.get(), 45, 45, SnapshotLimit::FromTail(5));
    cache_sticks = load(&cache, 45, 45, SnapshotLimit::FromTail(5));
    REQUIRE(cache_sticks == model_sticks);
  }, "test_coincidental_values_with_limits");
}
