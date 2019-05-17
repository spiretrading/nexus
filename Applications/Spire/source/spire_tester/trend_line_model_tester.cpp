#include <catch.hpp>
#include "spire/spire_tester/spire_tester.hpp"
#include "spire/charting/chart_point.hpp"
#include "spire/charting/chart_value.hpp"
#include "spire/charting/trend_line.hpp"
#include "spire/charting/trend_line_model.hpp"

using namespace Spire;

namespace {
  TrendLine make_line(int x1, int y1, int x2, int y2, const QColor& color,
      Qt::PenStyle style) {
    return TrendLine{{ChartPoint(ChartValue(x1), ChartValue(y1)),
      ChartPoint(ChartValue(x2), ChartValue(y2))}, color, style};
  }

  bool operator ==(const ChartPoint& lhs, const ChartPoint& rhs) {
    return lhs.m_x == rhs.m_x && lhs.m_y == rhs.m_y;
  }

  bool operator ==(const TrendLine& lhs, const TrendLine& rhs) {
    return std::get<0>(lhs.m_points) == std::get<0>(rhs.m_points) &&
      std::get<1>(lhs.m_points) == std::get<1>(rhs.m_points) &&
      lhs.m_color == rhs.m_color && lhs.m_style == rhs.m_style;
  }
}

TEST_CASE("test_creating_and_getting_trend_lines", "[TrendLineModel]") {
  auto model = TrendLineModel();
  auto line1 = make_line(10, 10, 20, 20, Qt::red, Qt::SolidLine);
  auto id1 = model.add(line1);
  auto a = model.get(id1);
  REQUIRE(line1 == model.get(id1));
  auto line2 = make_line(10, 10, 20, 20, Qt::red, Qt::SolidLine);
  auto id2 = model.add(line2);
  REQUIRE(line2 == model.get(id2));
  auto line3 = make_line(10, 10, 20, 20, Qt::red, Qt::SolidLine);
  auto id3 = model.add(line3);
  REQUIRE(line3 == model.get(id3));
  auto line4 = make_line(10, 10, 20, 20, Qt::red, Qt::SolidLine);
  auto id4 = model.add(line4);
  REQUIRE(line4 == model.get(id4));
  print_test_name("test_creating_and_getting_trend_lines");
}
