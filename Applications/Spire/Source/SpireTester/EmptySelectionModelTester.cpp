#include <doctest/doctest.h>
#include "Spire/Ui/EmptySelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("EmptySelectionModel") {
  TEST_CASE("construct") {
    auto model = EmptySelectionModel();
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS(model.get(0));
    REQUIRE_THROWS(model.set(0, 0));
    REQUIRE_THROWS(model.insert(0, 0));
    REQUIRE_THROWS(model.move(0, 0));
    REQUIRE_THROWS(model.remove(0));
  }
}
