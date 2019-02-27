#include <catch.hpp>
#include <QApplication>
#include <QTimer>
#include "spire/charting/cached_chart_model.hpp"
#include "spire/charting/chart_value.hpp"
#include "spire/charting/local_chart_model.hpp"

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

  template <typename T>
  void run_test(T&& test, const QString& name) {
    auto argc = 0;
    auto app = QCoreApplication(argc, nullptr);
    QTimer::singleShot(0,
      [&] {
        test();
        app.exit();
    });
    app.exec();
    qDebug() << "\n**************************************************";
    qDebug() << name << " PASSED";
    qDebug() << "**************************************************\n";
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
