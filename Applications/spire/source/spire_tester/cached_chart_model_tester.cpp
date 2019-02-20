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
        for(auto i = 101; i > 1; --i) {
          candlesticks.push_back({ChartValue(i * Money::ONE),
            ChartValue((i - 1) * Money::ONE)});
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
    qDebug() << "\n****************************************";
    qDebug() << name << " PASSED";
    qDebug() << "****************************************\n";
  }
}

TEST_CASE("test_right_no_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto model_sticks = wait(model->load(ChartValue(10 * Money::ONE),
      ChartValue(20 * Money::ONE)));
    auto cache = CachedChartModel(*model);
    auto cached_sticks = wait(cache.load(ChartValue(10 * Money::ONE),
      ChartValue(20 * Money::ONE)));
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = wait(model->load(ChartValue(30 * Money::ONE),
      ChartValue(40 * Money::ONE)));
    auto test_sticks = wait(cache.load(ChartValue(30 * Money::ONE),
      ChartValue(40 * Money::ONE)));
    REQUIRE(test_sticks == model_sticks);
  }, "test_right_no_overlap");
}

TEST_CASE("test_left_no_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto model_sticks = wait(model->load(ChartValue(20 * Money::ONE),
      ChartValue(30 * Money::ONE)));
    auto cache = CachedChartModel(*model);
    auto cached_sticks = wait(cache.load(ChartValue(20 * Money::ONE),
      ChartValue(30 * Money::ONE)));
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = wait(model->load(ChartValue(10 * Money::ONE),
      ChartValue(18 * Money::ONE)));
    auto test_sticks = wait(cache.load(ChartValue(10 * Money::ONE),
      ChartValue(18 * Money::ONE)));
    REQUIRE(test_sticks == model_sticks);
  }, "test_left_no_overlap");
}

TEST_CASE("test_right_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto model_sticks = wait(model->load(ChartValue(25 * Money::ONE),
      ChartValue(50 * Money::ONE)));
    auto cache = CachedChartModel(*model);
    auto cached_sticks = wait(cache.load(ChartValue(25 * Money::ONE),
      ChartValue(50 * Money::ONE)));
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = wait(model->load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE)));
    auto test_sticks = wait(cache.load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE)));
    REQUIRE(test_sticks == model_sticks);
  }, "test_right_overlap");
}

TEST_CASE("test_left_overlap", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto model_sticks = wait(model->load(ChartValue(25 * Money::ONE),
      ChartValue(50 * Money::ONE)));
    auto cache = CachedChartModel(*model);
    auto cached_sticks = wait(cache.load(ChartValue(25 * Money::ONE),
      ChartValue(50 * Money::ONE)));
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = wait(model->load(ChartValue(10 * Money::ONE),
      ChartValue(30 * Money::ONE)));
    auto test_sticks = wait(cache.load(ChartValue(10 * Money::ONE),
      ChartValue(30 * Money::ONE)));
    REQUIRE(test_sticks == model_sticks);
  }, "test_left_overlap");
}

TEST_CASE("test_subset", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto model_sticks = wait(model->load(ChartValue(25 * Money::ONE),
      ChartValue(75 * Money::ONE)));
    auto cache = CachedChartModel(*model);
    auto cached_sticks = wait(cache.load(ChartValue(25 * Money::ONE),
      ChartValue(75 * Money::ONE)));
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = wait(model->load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE)));
    auto test_sticks = wait(cache.load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE)));
    REQUIRE(test_sticks == model_sticks);
  }, "test_subset");
}

TEST_CASE("test_single_superset", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto model_sticks = wait(model->load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE)));
    auto cache = CachedChartModel(*model);
    auto cached_sticks = wait(cache.load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE)));
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = wait(model->load(ChartValue(20 * Money::ONE),
      ChartValue(80 * Money::ONE)));
    auto test_sticks = wait(cache.load(ChartValue(20 * Money::ONE),
      ChartValue(80 * Money::ONE)));
    REQUIRE(test_sticks == model_sticks);
  }, "test_single_superset");
}

TEST_CASE("test_multiple_supersets", "[CachedChartModel]") {
  run_test([=] {
    auto model = create_model();
    auto model_sticks = wait(model->load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE)));
    auto cache = CachedChartModel(*model);
    auto cached_sticks = wait(cache.load(ChartValue(40 * Money::ONE),
      ChartValue(60 * Money::ONE)));
    REQUIRE(model_sticks == cached_sticks);
    model_sticks = wait(model->load(ChartValue(20 * Money::ONE),
      ChartValue(80 * Money::ONE)));
    auto test_sticks = wait(cache.load(ChartValue(20 * Money::ONE),
      ChartValue(80 * Money::ONE)));
    REQUIRE(test_sticks == model_sticks);
    model_sticks = wait(model->load(ChartValue(10 * Money::ONE),
      ChartValue(90 * Money::ONE)));
    test_sticks = wait(cache.load(ChartValue(10 * Money::ONE),
      ChartValue(90 * Money::ONE)));
    REQUIRE(test_sticks == model_sticks);
  }, "test_multiple_supersets");
}
