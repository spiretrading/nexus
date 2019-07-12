#include <catch.hpp>
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Charting/ChartView.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

namespace {
  struct Gap {
    ChartValue m_start;
    ChartValue m_end;
  };

  const auto GAP_SIZE = 35;

  const auto TL() {
    static ChartPoint tl = {ChartValue(900), ChartValue(1)};
    return tl;
  }

  const auto BR() {
    static ChartPoint br = {ChartValue(950), ChartValue(0)};
    return br;
  }

  const auto BR_PIXEL = QPoint(100, 100);

  const auto GAPS() {
    static auto gaps = std::vector<Gap>();
    if(gaps.empty()) {
      gaps.push_back({ChartValue(910), ChartValue(920)});
      gaps.push_back({ChartValue(930), ChartValue(940)});
    }
    return gaps;
  }

  template<typename T, typename U>
  U map_to(T value, T a, T b, U c, U d) {
    return static_cast<U>((value - a) / (b - a) * (d - c) + c);
  }

  template<typename U>
  U map_to(int value, int a, int b, U c, U d) {
    return map_to(static_cast<double>(value), static_cast<double>(a),
      static_cast<double>(b), c, d);
  }

  template<typename T>
  int map_to(T value, T a, T b, int c, int d) {
    return static_cast<int>(
      map_to(value, a, b, static_cast<double>(c), static_cast<double>(d)));
  }

  QPoint to_pixel(const ChartPoint& point) {
    auto x = map_to(point.m_x, TL().m_x, BR().m_x, 0, BR_PIXEL.x());
    auto gaps = GAPS();
    for(auto& gap : gaps) {
      if(gap.m_start < point.m_x && gap.m_end > point.m_x) {
        auto new_x = to_pixel({gap.m_start, ChartValue()}).x() +
          static_cast<int>((point.m_x - gap.m_start) /
          (gap.m_end - gap.m_start) * static_cast<double>(GAP_SIZE));
        return {new_x, map_to(point.m_y, BR().m_y, TL().m_y, BR_PIXEL.y(), 0)};
      }
      if(point.m_x > gap.m_start) {
        auto gap_start = map_to(gap.m_start, TL().m_x, BR().m_x, 0,
          BR_PIXEL.x());
        auto gap_end = map_to(gap.m_end, TL().m_x, BR().m_x, 0, BR_PIXEL.x());
        x -= gap_end - gap_start - GAP_SIZE;
      }
    }
    return {x, map_to(point.m_y, BR().m_y, TL().m_y, BR_PIXEL.y(), 0)};
  }

  ChartPoint to_chart_point(const QPoint& point) {
    auto y = map_to(point.y(), BR_PIXEL.y(), 0, BR().m_y, TL().m_y);
    auto lower_x_pixel = 0;
    auto lower_x_chart_value = TL().m_x;
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
      return map_to(point.x(), lower_x_pixel, BR_PIXEL.x(),
        lower_x_chart_value, BR().m_x);
    }();
    return {x, y};
  }
}

TEST_CASE("test_to_chart_point", "[Charting]") {
  REQUIRE(to_chart_point({0, 0}) ==
    ChartPoint(ChartValue(900), ChartValue(1)));
  REQUIRE(to_chart_point({10, 0}) ==
    ChartPoint(ChartValue(905), ChartValue(1)));
  REQUIRE(to_chart_point({20, 0}) ==
    ChartPoint(ChartValue(910), ChartValue(1)));
  REQUIRE(to_chart_point({75, 0}) ==
    ChartPoint(ChartValue(930), ChartValue(1)));
  REQUIRE(to_chart_point({110, 0}) ==
    ChartPoint(ChartValue(940), ChartValue(1)));
  print_test_name("test_to_chart_point");
}

TEST_CASE("test_to_pixel", "[Charting]") {
  REQUIRE(to_pixel({ChartValue(900), ChartValue(1)}) == QPoint(0, 0));
  auto a = to_pixel({ChartValue(930), ChartValue(1)});
  REQUIRE(to_pixel({ChartValue(930), ChartValue(1)}) == QPoint(75, 0));
  print_test_name("test_to_pixel");
}
