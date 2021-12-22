#include <doctest/doctest.h>
#include "Spire/Spire/LocalCompositeValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  struct Point {
    int m_x;
    int m_y;
  };
}

TEST_SUITE("LocalCompositeValueModel") {
  TEST_CASE("construct") {
    auto point = std::make_shared<LocalCompositeValueModel<Point>>();
    auto x = point->get(&Point::m_x);
  }
}
