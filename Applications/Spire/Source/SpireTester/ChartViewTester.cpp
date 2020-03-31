#include <catch2/catch.hpp>
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/Charting/ChartView.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  struct Gap {
    Scalar m_start;
    Scalar m_end;
  };

  const auto GAP_SIZE = 35;

  const auto TOP_LEFT() {
    static auto top_left = ChartPoint(Scalar(900), Scalar(1));
    return top_left;
  }

  const auto BOTTOM_RIGHT() {
    static auto bottom_right = ChartPoint(Scalar(950), Scalar(0));
    return bottom_right;
  }

  const auto ADJUSTED_BOTTOM_RIGHT() {
    static auto adjusted_bottom_right = ChartPoint(Scalar(952.5), Scalar(0));
    return adjusted_bottom_right;
  }

  const auto BOTTOM_RIGHT_PIXEL = QPoint(200, 100);

  const auto GAPS() {
    static auto gaps = std::vector<Gap>();
    if(gaps.empty()) {
      gaps.push_back({Scalar(910), Scalar(920)});
      gaps.push_back({Scalar(930), Scalar(940)});
    }
    return gaps;
  }

  QPoint to_pixel(const ChartPoint& point) {
    auto x = map_to(point.m_x, TOP_LEFT().m_x, BOTTOM_RIGHT().m_x, 0,
      BOTTOM_RIGHT_PIXEL.x());
    auto gaps = GAPS();
    for(auto& gap : gaps) {
      if(gap.m_start < point.m_x && gap.m_end > point.m_x) {
        auto new_x = to_pixel({gap.m_start, Scalar()}).x() +
          static_cast<int>((point.m_x - gap.m_start) /
          (gap.m_end - gap.m_start) * static_cast<double>(GAP_SIZE));
        return {new_x, map_to(point.m_y, BOTTOM_RIGHT().m_y, TOP_LEFT().m_y,
          BOTTOM_RIGHT_PIXEL.y(), 0)};
      }
      if(point.m_x > gap.m_start) {
        auto gap_start = map_to(gap.m_start, TOP_LEFT().m_x,
          BOTTOM_RIGHT().m_x, 0, BOTTOM_RIGHT_PIXEL.x());
        auto gap_end = map_to(gap.m_end, TOP_LEFT().m_x, BOTTOM_RIGHT().m_x, 0,
          BOTTOM_RIGHT_PIXEL.x());
        x -= gap_end - gap_start - GAP_SIZE;
      }
    }
    return {x, map_to(point.m_y, BOTTOM_RIGHT().m_y, TOP_LEFT().m_y,
      BOTTOM_RIGHT_PIXEL.y(), 0)};
  }

  ChartPoint to_chart_point(const QPoint& point) {
    auto y = map_to(point.y(), BOTTOM_RIGHT_PIXEL.y(), 0, BOTTOM_RIGHT().m_y,
      TOP_LEFT().m_y);
    auto lower_x_pixel = 0;
    auto lower_x_chart_value = TOP_LEFT().m_x;
    auto x = [&] {
      for(auto gap : GAPS()) {
        auto gap_start_pixel = to_pixel({gap.m_start, y}).x();
        if(point.x() <= gap_start_pixel) {
          return map_to(point.x(), lower_x_pixel, gap_start_pixel,
            lower_x_chart_value, gap.m_start);
        }
        auto gap_end_pixel = to_pixel({gap.m_end, y}).x();
        if(point.x() < gap_end_pixel) {
          return map_to(point.x(), gap_start_pixel, gap_end_pixel,
            gap.m_start, gap.m_end);
        }
        lower_x_pixel = gap_end_pixel;
        lower_x_chart_value = gap.m_end;
      }
      return map_to(point.x(), lower_x_pixel, BOTTOM_RIGHT_PIXEL.x(),
        lower_x_chart_value, ADJUSTED_BOTTOM_RIGHT().m_x);
    }();
    return {x, y};
  }

  bool test_pixel_to_point(int x_pixel, double expected_value) {
    auto cv_x = to_chart_point({x_pixel, 0}).m_x;
    return Truncate(static_cast<Money>(cv_x), 6) ==
      Truncate(Money(expected_value), 6);
  }
}

TEST_CASE("test_to_chart_point", "[Charting]") {
  REQUIRE(test_pixel_to_point(0, 900));
  REQUIRE(test_pixel_to_point(10, 902.5));
  REQUIRE(test_pixel_to_point(20, 905));
  REQUIRE(test_pixel_to_point(30, 907.5));
  REQUIRE(test_pixel_to_point(40, 910));
  REQUIRE(test_pixel_to_point(50, 910 + 10.0 / 35.0 * 10.0));
  REQUIRE(test_pixel_to_point(60, 910 + 20.0 / 35.0 * 10.0));
  REQUIRE(test_pixel_to_point(70, 910 + 30.0 / 35.0 * 10.0));
  REQUIRE(test_pixel_to_point(80, 921.25));
  REQUIRE(test_pixel_to_point(90, 923.75));
  REQUIRE(test_pixel_to_point(100, 926.25));
  REQUIRE(test_pixel_to_point(110, 928.75));
  REQUIRE(test_pixel_to_point(120, 930 + 5.0 / 35.0 * 10.0));
  REQUIRE(test_pixel_to_point(130, 930 + 15.0 / 35.0 * 10.0));
  REQUIRE(test_pixel_to_point(140, 930 + 25.0 / 35.0 * 10.0));
  REQUIRE(test_pixel_to_point(150, 940));
  REQUIRE(test_pixel_to_point(160, 942.5));
  REQUIRE(test_pixel_to_point(170, 945));
  REQUIRE(test_pixel_to_point(180, 947.5));
  REQUIRE(test_pixel_to_point(190, 950));
  REQUIRE(test_pixel_to_point(200, 952.5));
  print_test_name("test_to_chart_point");
}

TEST_CASE("test_to_pixel", "[Charting]") {
  REQUIRE(to_pixel({Scalar(900), Scalar(1)}) == QPoint(0, 0));
  REQUIRE(to_pixel({Scalar(905), Scalar(1)}) == QPoint(20, 0));
  REQUIRE(to_pixel({Scalar(910), Scalar(1)}) == QPoint(40, 0));
  REQUIRE(to_pixel({Scalar(915), Scalar(1)}) == QPoint(57, 0));
  REQUIRE(to_pixel({Scalar(920), Scalar(1)}) == QPoint(75, 0));
  REQUIRE(to_pixel({Scalar(925), Scalar(1)}) == QPoint(95, 0));
  REQUIRE(to_pixel({Scalar(930), Scalar(1)}) == QPoint(115, 0));
  REQUIRE(to_pixel({Scalar(935), Scalar(1)}) == QPoint(132, 0));
  REQUIRE(to_pixel({Scalar(940), Scalar(1)}) == QPoint(150, 0));
  REQUIRE(to_pixel({Scalar(945), Scalar(1)}) == QPoint(170, 0));
  REQUIRE(to_pixel({Scalar(950), Scalar(1)}) == QPoint(190, 0));
  REQUIRE(to_pixel({Scalar(952.5), Scalar(1)}) == QPoint(200, 0));
  print_test_name("test_to_pixel");
}
