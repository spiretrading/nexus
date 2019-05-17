#include <catch.hpp>
#include "spire/spire_tester/spire_tester.hpp"
#include "spire/charting/chart_point.hpp"
#include "spire/charting/chart_value.hpp"
#include "spire/charting/trend_line.hpp"
#include "spire/charting/trend_line_model.hpp"

using namespace Spire;

namespace {
  ChartPoint make_point(int x, int y) {
    return ChartPoint(ChartValue(x), ChartValue(y));
  }  

  TrendLine make_line(int x1, int y1, int x2, int y2, const QColor& color,
      Qt::PenStyle style) {
    return TrendLine{{make_point(x1, y1), make_point(x2, y2)}, color, style};
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
  auto line2 = make_line(30, 30, 40, 40, Qt::red, Qt::SolidLine);
  auto id2 = model.add(line2);
  auto line3 = make_line(50, 50, 60, 60, Qt::red, Qt::SolidLine);
  auto id3 = model.add(line3);
  auto line4 = make_line(70, 70, 80, 80, Qt::red, Qt::SolidLine);
  auto id4 = model.add(line4);
  REQUIRE(line1 == model.get(id1));
  REQUIRE(line2 == model.get(id2));
  REQUIRE(line3 == model.get(id3));
  REQUIRE(line4 == model.get(id4));
  print_test_name("test_creating_and_getting_trend_lines");
}

TEST_CASE("test_updating_and_getting_trend_lines", "[TrendLineModel]") {
  auto model = TrendLineModel();
  auto line1 = make_line(10, 10, 20, 20, Qt::red, Qt::SolidLine);
  auto id1 = model.add(line1);
  auto line2 = make_line(30, 30, 40, 40, Qt::red, Qt::SolidLine);
  auto id2 = model.add(line2);
  auto line3 = make_line(50, 50, 60, 60, Qt::red, Qt::SolidLine);
  auto id3 = model.add(line3);
  auto updated_line1 = model.get(id1);
  updated_line1.m_points = {make_point(110, 110), make_point(120, 120)};
  model.update(updated_line1, id1);
  auto updated_line2 = model.get(id2);
  updated_line2.m_points = {make_point(130, 130), make_point(140, 140)};
  model.update(updated_line2, id2);
  auto updated_line3 = model.get(id3);
  updated_line3.m_points = {make_point(150, 150), make_point(160, 160)};
  model.update(updated_line3, id3);
  REQUIRE(updated_line1 == model.get(id1));
  REQUIRE(updated_line2 == model.get(id2));
  REQUIRE(updated_line3 == model.get(id3));
  print_test_name("test_updating_and_getting_trend_lines");
}
