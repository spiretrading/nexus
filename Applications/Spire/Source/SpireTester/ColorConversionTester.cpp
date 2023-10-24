#include <doctest/doctest.h>
#include "Spire/Ui/ColorConversion.hpp"

using namespace Spire;

namespace {
  const auto EPSILON = 0.001;

  void require_rgb(const QColor& lhs, const QColor& rhs) {
    REQUIRE(lhs.isValid() == rhs.isValid());
    REQUIRE(lhs.name() == rhs.name());
  }

  void require_oklab(const OklabColor& lhs, const OklabColor& rhs) {
    REQUIRE(lhs.m_is_valid == rhs.m_is_valid);
    REQUIRE(std::abs(lhs.m_l - rhs.m_l) < EPSILON);
    REQUIRE(std::abs(lhs.m_a - rhs.m_a) < EPSILON);
    REQUIRE(std::abs(lhs.m_b - rhs.m_b) < EPSILON);
  }

  void require_oklch(const OklchColor& lhs, const OklchColor& rhs) {
    REQUIRE(lhs.m_is_valid == rhs.m_is_valid);
    REQUIRE(std::abs(lhs.m_l - rhs.m_l) < EPSILON);
    REQUIRE(std::abs(lhs.m_c - rhs.m_c) < EPSILON);
    REQUIRE(std::abs(lhs.m_h - rhs.m_h) < EPSILON);
  }

  void require_apca(double lhs, double rhs) {
    REQUIRE(std::abs(lhs - rhs) < EPSILON);
  }
}

TEST_SUITE("ColorConversion") {
  TEST_CASE("rgb_to_oklab") {
    auto rgb = QColor();
    require_oklab(to_oklab(rgb), OklabColor{false, 0, 0, 0});
    rgb = QColor(Qt::black);
    require_oklab(to_oklab(rgb), OklabColor{true, 0, 0, 0});
    rgb = QColor(0xFFFFFF);
    require_oklab(to_oklab(rgb), OklabColor{true, 1, 0, 0});
    rgb = QColor(0x0000FF);
    require_oklab(to_oklab(rgb), OklabColor{true, 0.45201, -0.03246, -0.31153});
    rgb = QColor(0x00FF00);
    require_oklab(to_oklab(rgb), OklabColor{true, 0.86644, -0.23389, 0.1795});
  }

  TEST_CASE("oklab_to_rgb") {
    auto oklab = OklabColor{false, 0, 0, 0};
    require_rgb(to_rgb(oklab), QColor());
    oklab = OklabColor{true, 1, 0, 0};
    require_rgb(to_rgb(oklab), QColor(0xFFFFFF));
    oklab = OklabColor{true, 0.45201, -0.03246, -0.31153};
    require_rgb(to_rgb(oklab), QColor(0x0000FF));
    oklab = OklabColor{true, 0.86644, -0.23389, 0.1795};
    require_rgb(to_rgb(oklab), QColor(0x00FF00));
    auto rgb = QColor(0xFFFF00);
    require_rgb(to_rgb(to_oklab(rgb)), rgb);
  }

  TEST_CASE("rgb_to_oklch") {
    auto rgb = QColor();
    require_oklch(to_oklch(rgb), OklchColor{false, 0, 0, 0});
    rgb = QColor(Qt::black);
    require_oklch(to_oklch(rgb), OklchColor{true, 0, 0, 0});
    rgb = QColor(0xFFFFFF);
    require_oklch(to_oklch(rgb), OklchColor{true, 1, 0, 0});
    rgb = QColor(0x0000FF);
    require_oklch(to_oklch(rgb), OklchColor{true, 0.45201, 0.31321, 264.05203});
    rgb = QColor(0x00FF00);
    require_oklch(to_oklch(rgb), OklchColor{true, 0.86644, 0.29483, 142.49535});
  }

  TEST_CASE("oklch_to_rgb") {
    auto oklab = OklchColor{false, 0, 0, 0};
    require_rgb(to_rgb(oklab), QColor());
    oklab = OklchColor{true, 1, 0, 0};
    require_rgb(to_rgb(oklab), QColor(0xFFFFFF));
    oklab = OklchColor{true, 0.45201, 0.31321, 264.05203};
    require_rgb(to_rgb(oklab), QColor(0x0000FF));
    oklab = OklchColor{true, 0.86644, 0.29483, 142.49535};
    require_rgb(to_rgb(oklab), QColor(0x00FF00));
    auto rgb = QColor(0xFFFF00);
    require_rgb(to_rgb(to_oklch(rgb)), rgb);
  }

  TEST_CASE("full_conversion") {
    auto rgb = QColor(0xABCD12);
    require_rgb(to_rgb(to_oklab(to_rgb(to_oklch(rgb)))), rgb);
  }

  TEST_CASE("APCA") {
    require_apca(apca(QColor(0x888888), QColor(0xFFFFFF)), 63.05647);
    require_apca(apca(QColor(0xFFFFFF), QColor(0x888888)), -68.54146);
    require_apca(apca(QColor(Qt::black), QColor(0xAAAAAA)), 58.14626);
    require_apca(apca(QColor(0xAAAAAA), QColor(Qt::black)), -56.24113);
    require_apca(apca(QColor(0x112233), QColor(0xDDEEFF)), 91.66831);
    require_apca(apca(QColor(0xDDEEFF), QColor(0x112233)), -93.06770);
    require_apca(apca(QColor(0x112233), QColor(0x444444)), 8.32326);
    require_apca(apca(QColor(0x444444), QColor(0x112233)), -7.52688);
  }
}
