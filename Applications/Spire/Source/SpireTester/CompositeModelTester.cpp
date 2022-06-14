#include <doctest/doctest.h>
#include "Spire/Spire/CompositeModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

namespace {
  struct Point {
    int m_x;
    int m_y;
  };
}

TEST_SUITE("CompositeModel") {
  TEST_CASE("access") {
    auto x = std::make_shared<LocalValueModel<int>>();
    auto y = std::make_shared<LocalValueModel<int>>();
    auto model = std::make_shared<CompositeModel<Point>>();
    model->define(&Point::m_x, x);
    model->define(&Point::m_y, y);
  }
}
