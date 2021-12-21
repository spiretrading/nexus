#include <doctest/doctest.h>
#include "Spire/Spire/LocalFieldModel.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  struct Point {
    int m_x;
    int m_y;
  };
}

TEST_SUITE("LocalFieldModel") {
  TEST_CASE("construct") {
/*
    auto point = std::make_shared<LocalFieldModel<Point>>();
    auto x = point->get(&Point::m_x);
*/
  }
}
