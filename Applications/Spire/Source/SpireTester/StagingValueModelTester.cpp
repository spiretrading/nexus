#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/StagingValueModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("StagingValueModel") {
  TEST_CASE("stage_local_model") {
    auto source = std::make_shared<LocalValueModel<int>>(47);
    auto model = StagingValueModel(source);
    REQUIRE(model.get_state() == QValidator::State::Acceptable);
    REQUIRE(model.get() == 47);
    REQUIRE(model.set(123) == QValidator::State::Acceptable);
    REQUIRE(model.get() == 123);
    REQUIRE(model.get_state() == QValidator::State::Intermediate);
    REQUIRE(source->get() == 47);
    REQUIRE(model.commit() == QValidator::State::Acceptable);
    REQUIRE(model.get_state() == QValidator::State::Acceptable);
    REQUIRE(source->get() == 123);
    REQUIRE(model.set(321) == QValidator::State::Acceptable);
    source->set(555);
    REQUIRE(model.get() == 555);
    REQUIRE(model.get_state() == QValidator::State::Acceptable);
  }
}
