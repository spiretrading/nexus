#include <doctest/doctest.h>
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  struct Point {
    int m_x;
    int m_y;
  };
}

TEST_SUITE("FieldValueModel") {
  TEST_CASE("construct") {
    auto source = std::make_shared<LocalValueModel<Point>>(Point(10, 20));
    auto x = make_field_value_model(source, &Point::m_x);
    auto y = make_field_value_model(source, &Point::m_y);
    REQUIRE(x->get() == 10);
    REQUIRE(y->get() == 20);
    REQUIRE(x->set(5) == QValidator::State::Acceptable);
    REQUIRE(source->get().m_x == 5);
  }
}
