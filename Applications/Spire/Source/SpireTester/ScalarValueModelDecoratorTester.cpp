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
    REQUIRE(model.get() == 47);
    REQUIRE(model.get_minimum() == std::numeric_limits<int>::lowest());
    REQUIRE(model.get_maximum() == std::numeric_limits<int>::max());
    REQUIRE(model.get_increment() == none);
    model.set_minimum(39);
    model.set_maximum(41);
    REQUIRE(model.get_minimum() == 39);
    REQUIRE(model.get_maximum() == 41);
    REQUIRE(model.set(3) == QValidator::State::Intermediate);
    REQUIRE(model.get() == 3);
    REQUIRE(model.set(42) == QValidator::State::Intermediate);
    REQUIRE(model.get() == 42);
  }

  TEST_CASE("decorate_optional_model") {
    auto source = std::make_shared<LocalValueModel<optional<int>>>();
    auto model = ScalarValueModelDecorator(source);
    REQUIRE(model.get_state() == QValidator::State::Acceptable);
    REQUIRE(model.get() == none);
    REQUIRE(model.get_increment() == none);
    REQUIRE(model.set(3) == QValidator::State::Acceptable);
    REQUIRE(model.get() == 3);
    REQUIRE(model.set(none) == QValidator::State::Acceptable);
    REQUIRE(model.get() == none);
  }
}
