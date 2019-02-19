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
        for(auto i = 1; i < 101; ++i) {
          candlesticks.push_back({ChartValue(i * Money::ONE),
            ChartValue((i + 1) * Money::ONE)});
        }
        return candlesticks;
      }());
    return local_model;
  }
}

TEST_CASE("test_right_no_overlap", "[CachedChartModel]") {
  auto argc = 0;
  auto app = QCoreApplication(argc, nullptr);
  QTimer::singleShot(0,
    [&] {
      auto model = create_model();
      auto first = ChartValue(10 * Money::ONE);
      auto last = ChartValue(20 * Money::ONE);
      auto model_sticks = wait(model->load(first, last));
      auto cache = CachedChartModel(*model);
      auto cached_sticks = wait(cache.load(first, last));
      REQUIRE(model_sticks == cached_sticks);
      app.exit();
  });
  app.exec();
}

TEST_CASE("test_left_no_overlap", "[CachedChartModel]") {

}

TEST_CASE("test_right_overlap", "[CachedChartModel]") {

}

TEST_CASE("test_left_overlap", "[CachedChartModel]") {

}

TEST_CASE("test_subset", "[CachedChartModel]") {

}

TEST_CASE("test_single_superset", "[CachedChartModel]") {

}

TEST_CASE("test_multiple_supersets", "[CachedChartModel]") {

}
