#include <doctest/doctest.h>
#include "Spire/Ui/SingleSelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("SingleSelectionModel") {
  TEST_CASE("construct") {
    auto model = SingleSelectionModel();
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS(model.get(0));
    REQUIRE_THROWS(model.set(0, 54));
    REQUIRE_THROWS(model.move(0, 0));
    REQUIRE_THROWS(model.remove(0));
  }

  TEST_CASE("update") {
    auto model = SingleSelectionModel();
    REQUIRE(model.push(5) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0) == 5);
    REQUIRE(model.set(0, 12) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0) == 12);
    REQUIRE(model.push(42) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0) == 42);
    REQUIRE(model.insert(225, 0) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0) == 225);
    REQUIRE(model.remove(0) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 0);
  }
}
