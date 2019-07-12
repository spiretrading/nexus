#include <catch.hpp>
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Charting/ChartView.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto GAP_SIZE() {
    static auto size = scale_width(35);
    return size;
  }

  const auto TL() {
    static ChartPoint tl = {ChartValue(900 * Money::ONE),
      ChartValue(Money::ONE)};
    return tl;
  }

  const auto BR() {
    static ChartPoint br = {ChartValue(950 * Money::ONE),
      ChartValue(0)};
    return br;
  }

  const auto BR_PIXEL = QPoint(561, 311);

  const auto GAPS() {
    static auto gaps = std::vector<Gap>();
    gaps.push_back({ChartValue(940 * Money::ONE),
      ChartValue(950 * Money::ONE)});
    gaps.push_back({ChartValue(960 * Money::ONE),
      ChartValue(965 * Money::ONE)});
    return gaps;
  }

  struct Gap {
    ChartValue m_start;
    ChartValue m_end;
  };

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

  ChartPoint to_chart_point(const QPoint& point) {
    auto y = map_to(point.y(), BR_PIXEL.y(), 0, BR().m_y, TL().m_y);
    auto lower_x_pixel = 0;
    auto lower_x_chart_value = TL().m_x;
    auto x = [&] {
      for(auto& gap : GAPS()) {
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

  QPoint to_pixel(const ChartPoint& point) {
    auto x = map_to(point.m_x, TL().m_x, BR().m_x, 0, BR_PIXEL.x());
    for(auto& gap : GAPS()) {
      if(gap.m_start < point.m_x && gap.m_end > point.m_x) {
        auto new_x = to_pixel({gap.m_start, ChartValue()}).x() +
          static_cast<int>((point.m_x - gap.m_start) /
          (gap.m_end - gap.m_start) * static_cast<double>(GAP_SIZE()));
        return {new_x, map_to(point.m_y, BR().m_y, TL().m_y, BR_PIXEL.y(), 0)};
      }
      if(point.m_x > gap.m_start) {
        auto gap_start = map_to(gap.m_start, TL().m_x, BR().m_x, 0,
          BR_PIXEL.x());
        auto gap_end = map_to(gap.m_end, TL().m_x, BR().m_x, 0, BR_PIXEL.x());
        x -= gap_end - gap_start - GAP_SIZE();
      }
    }
    return {x, map_to(point.m_y, BR().m_y, TL().m_y, BR_PIXEL.y(), 0)};
  }
}

TEST_CASE("test_to_pixel", "[Charting]") {
  print_test_name("test_to_pixel");
}

TEST_CASE("test_to_chart_point", "") {
  print_test_name("test_to_chart_point");
}
