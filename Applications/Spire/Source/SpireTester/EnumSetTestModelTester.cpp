#include <doctest/doctest.h>
#include "Spire/Spire/EnumSetTestModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  BEAM_ENUM(Test,
    ALPHA,
    BETA,
    GAMMA,
    KAPPA);
}

TEST_SUITE("EnumSetTestModel") {
  TEST_CASE("set_and_get") {
    auto enum_model = std::make_shared<LocalValueModel<EnumSet<Test>>>();
    auto gamma_model = make_enum_set_test_model(enum_model, Test::GAMMA);
    REQUIRE(!gamma_model->get());
    auto e = enum_model->get();
    e.Set(Test::GAMMA);
    enum_model->set(e);
    REQUIRE(gamma_model->get());
  }
}
