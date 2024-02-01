#include <doctest/doctest.h>
#include "Spire/Ui/BoxGeometry.hpp"

using namespace Spire;

namespace {
  void require_borders(const BoxGeometry& geometry,
      const std::tuple<int, int, int, int>& borders) {
    REQUIRE(geometry.get_border_top() == std::get<0>(borders));
    REQUIRE(geometry.get_border_right() == std::get<1>(borders));
    REQUIRE(geometry.get_border_bottom() == std::get<2>(borders));
    REQUIRE(geometry.get_border_left() == std::get<3>(borders));
  }

  void require_padding(const BoxGeometry& geometry,
      const std::tuple<int, int, int, int>& padding) {
    REQUIRE(geometry.get_padding_top() == std::get<0>(padding));
    REQUIRE(geometry.get_padding_right() == std::get<1>(padding));
    REQUIRE(geometry.get_padding_bottom() == std::get<2>(padding));
    REQUIRE(geometry.get_padding_left() == std::get<3>(padding));
  }
}

TEST_SUITE("BoxGeometry") {
  TEST_CASE("construction") {
    auto geometry = BoxGeometry();
    REQUIRE(geometry.get_border_area() == QRect(QPoint(0, 0), QSize(0, 0)));
    require_borders(geometry, {0, 0, 0, 0});
    REQUIRE(geometry.get_padding_area() == QRect(QPoint(0, 0), QSize(0, 0)));
    require_padding(geometry, {0, 0, 0, 0});
    REQUIRE(geometry.get_content_area() == QRect(QPoint(0, 0), QSize(0, 0)));
    REQUIRE(geometry.get_geometry() == QRect(QPoint(0, 0), QSize(0, 0)));
  }

  TEST_CASE("set_areas") {
    auto geometry = BoxGeometry();
    geometry.set_size(QSize(100, 100));
    REQUIRE(geometry.get_border_area() == QRect(QPoint(0, 0), QSize(100, 100)));
    require_borders(geometry, {0, 0, 0, 0});
    REQUIRE(
      geometry.get_padding_area() == QRect(QPoint(0, 0), QSize(100, 100)));
    require_padding(geometry, {0, 0, 0, 0});
    REQUIRE(
      geometry.get_content_area() == QRect(QPoint(0, 0), QSize(100, 100)));
    geometry.set_border_left(10);
    geometry.set_border_bottom(20);
    geometry.set_border_right(3);
    geometry.set_border_top(12);
    require_borders(geometry, {12, 3, 20, 10});
    require_padding(geometry, {0, 0, 0, 0});
    REQUIRE(
      geometry.get_padding_area() == QRect(QPoint(10, 12), QSize(87, 68)));
    REQUIRE(
      geometry.get_content_area() == QRect(QPoint(10, 12), QSize(87, 68)));
    geometry.set_padding_right(6);
    geometry.set_padding_left(14);
    geometry.set_padding_top(1);
    geometry.set_padding_bottom(9);
    require_borders(geometry, {12, 3, 20, 10});
    require_padding(geometry, {1, 6, 9, 14});
    REQUIRE(
      geometry.get_padding_area() == QRect(QPoint(10, 12), QSize(87, 68)));
    REQUIRE(
      geometry.get_content_area() == QRect(QPoint(24, 13), QSize(67, 58)));
    geometry.set_size(QSize(200, 100));
    require_borders(geometry, {12, 3, 20, 10});
    require_padding(geometry, {1, 6, 9, 14});
    REQUIRE(
      geometry.get_padding_area() == QRect(QPoint(10, 12), QSize(187, 68)));
    REQUIRE(
      geometry.get_content_area() == QRect(QPoint(24, 13), QSize(167, 58)));
    geometry.set_size(QSize(200, 200));
    require_borders(geometry, {12, 3, 20, 10});
    require_padding(geometry, {1, 6, 9, 14});
    REQUIRE(
      geometry.get_padding_area() == QRect(QPoint(10, 12), QSize(187, 168)));
    REQUIRE(
      geometry.get_content_area() == QRect(QPoint(24, 13), QSize(167, 158)));
    geometry.set_size(QSize(50, 200));
    require_borders(geometry, {12, 3, 20, 10});
    require_padding(geometry, {1, 6, 9, 14});
    REQUIRE(
      geometry.get_padding_area() == QRect(QPoint(10, 12), QSize(37, 168)));
    REQUIRE(
      geometry.get_content_area() == QRect(QPoint(24, 13), QSize(17, 158)));
    geometry.set_size(QSize(50, 50));
    require_borders(geometry, {12, 3, 20, 10});
    require_padding(geometry, {1, 6, 9, 14});
    REQUIRE(
      geometry.get_padding_area() == QRect(QPoint(10, 12), QSize(37, 18)));
    REQUIRE(
      geometry.get_content_area() == QRect(QPoint(24, 13), QSize(17, 8)));
  }
}
