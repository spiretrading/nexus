#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("ScalarValueModelDecorator") {
  TEST_CASE("decorate_local_model") {
    auto source = std::make_shared<LocalValueModel<int>>(47);
    auto model = ScalarValueModelDecorator(source);
    REQUIRE(model.get_state() == QValidator::State::Acceptable);
    REQUIRE(model.get_current() == 47);
    REQUIRE(model.get_minimum() == std::numeric_limits<int>::lowest());
    REQUIRE(model.get_maximum() == std::numeric_limits<int>::max());
    REQUIRE(model.get_increment() == 1);
    model.set_minimum(100);
    model.set_maximum(200);
  }
}
