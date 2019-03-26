#include <catch.hpp>
#include "spire/charting/cached_chart_model.hpp"
#include "spire/charting/chart_value.hpp"
#include "spire/charting/local_chart_model.hpp"
#include "spire/spire_tester/spire_tester.hpp"
#include "spire/spire_tester/test_chart_model.hpp"

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

  std::vector<Candlestick> load(ChartModel* model, int first, int last) {
    return std::move(wait(model->load(ChartValue(first * Money::ONE),
      ChartValue(last * Money::ONE))));
  }
}

TEST_CASE("test_right_no_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 10, 20);
    auto cached_sticks = load(&cache, 10, 20);
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 30, 40);
    auto test_sticks = load(&cache, 30, 40);
    REQUIRE(test_sticks == model_sticks);
  }, "test_right_no_overlap");
}

TEST_CASE("test_left_no_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 20, 30);
    auto cached_sticks = load(&cache, 20, 30);
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 10, 18);
    auto test_sticks = load(&cache, 10, 18);
    REQUIRE(test_sticks == model_sticks);
  }, "test_left_no_overlap");
}

TEST_CASE("test_right_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 25, 50);
    auto cached_sticks = load(&cache, 25, 50);
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 40, 60);
    auto test_sticks = load(&cache, 40, 60);
    REQUIRE(test_sticks == model_sticks);
  }, "test_right_overlap");
}

TEST_CASE("test_left_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 25, 50);
    auto cached_sticks = load(&cache, 25, 50);
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 10, 30);
    auto test_sticks = load(&cache, 10, 30);
    REQUIRE(test_sticks == model_sticks);
  }, "test_left_overlap");
}

TEST_CASE("test_subset", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 25, 75);
    auto cached_sticks = load(&cache, 25, 75);
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 40, 60);
    auto test_sticks = load(&cache, 40, 60);
    REQUIRE(test_sticks == model_sticks);
  }, "test_subset");
}

TEST_CASE("test_single_superset", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 40, 60);
    auto cached_sticks = load(&cache, 40, 60);
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 20, 80);
    auto test_sticks = load(&cache, 20, 80);
    REQUIRE(test_sticks == model_sticks);
  }, "test_single_superset");
}

TEST_CASE("test_multiple_supersets", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 40, 60);
    auto cached_sticks = load(&cache, 40, 60);
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 20, 80);
    auto test_sticks = load(&cache, 20, 80);
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 10, 90);
    test_sticks = load(&cache, 10, 90);
    REQUIRE(test_sticks == model_sticks);
  }, "test_multiple_supersets");
}

TEST_CASE("test_multiple_subsets_and_supersets", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto cache = CachedChartModel(*model);
    auto model_sticks = load(model.get(), 40, 50);
    auto cached_sticks = load(&cache, 40, 50);
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = load(model.get(), 60, 70);
    auto test_sticks = load(&cache, 60, 70);
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 80, 90);
    test_sticks = load(&cache, 80, 90);
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 45, 72);
    test_sticks = load(&cache, 45, 72);
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 35, 95);
    test_sticks = load(&cache, 35, 95);
    REQUIRE(test_sticks == model_sticks);
    model_sticks = load(model.get(), 40, 60);
    test_sticks = load(&cache, 40, 60);
    REQUIRE(test_sticks == model_sticks);
  }, "test_multiple_subsets_and_supersets");
}

TEST_CASE("test_cache_functionality", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto test_model = TestChartModel(model->get_x_axis_type(),
      model->get_y_axis_type());
    auto cache = CachedChartModel(test_model);
    auto pre_load1 = cache.load(ChartValue(40 * Money::ONE),
      ChartValue(50 * Money::ONE));
    test_model.pop_load()->set_result(wait(
      model->load(ChartValue(40 * Money::ONE), ChartValue(50 * Money::ONE))));
    wait(std::move(pre_load1));
    auto pre_load2 = cache.load(ChartValue(60 * Money::ONE),
      ChartValue(70 * Money::ONE));
    test_model.pop_load()->set_result(wait(
      model->load(ChartValue(60 * Money::ONE), ChartValue(70 * Money::ONE))));
    wait(std::move(pre_load2));
    auto cache_test_load = cache.load(ChartValue(45 * Money::ONE),
      ChartValue(75 * Money::ONE));
    auto cache_load1 = test_model.pop_load();
    REQUIRE(cache_load1->get_first() == ChartValue(50 * Money::ONE));
    REQUIRE(cache_load1->get_last() == ChartValue(60 * Money::ONE));
    auto cache_load2 = test_model.pop_load();
    REQUIRE(cache_load2->get_first() == ChartValue(70 * Money::ONE));
    REQUIRE(cache_load2->get_last() == ChartValue(75 * Money::ONE));
    cache_load1->set_result({});
    cache_load2->set_result({});
  }, "test_cache_functionality");
}

TEST_CASE("test_load_ordering_b_before_a", "[CachedChartModel") {
  run_test([=] {
    auto model = create_model();
    auto test_model = TestChartModel(model->get_x_axis_type(),
      model->get_y_axis_type());
    auto cache = CachedChartModel(test_model);
    auto load_a = cache.load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE));
    auto load_b = cache.load(ChartValue(50 * Money::ONE),
      ChartValue(70 * Money::ONE));
    auto entry_a = test_model.pop_load();
    auto entry_b = test_model.pop_load();
    entry_b->set_result(wait(model->load(
      ChartValue(50 * Money::ONE), ChartValue(70 * Money::ONE))));
    entry_a->set_result(wait(model->load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE))));
    auto result_a = wait(std::move(load_a));
    auto result_b = wait(std::move(load_b));
    REQUIRE(result_a == wait(model->load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE))));
    REQUIRE(result_b == wait(model->load(ChartValue(50 * Money::ONE),
      ChartValue(70 * Money::ONE))));
  }, "test_load_ordering_b_then_a");
}


