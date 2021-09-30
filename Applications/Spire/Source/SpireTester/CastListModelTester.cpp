#include <doctest/doctest.h>
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/CastListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("CastListModel") {
  TEST_CASE("get") {
    auto source = std::make_shared<ArrayListModel>();
    auto model = CastListModel<int>(source);
    source->push(123);
    REQUIRE(model.get(0) == 123);
  }

  TEST_CASE("set") {
    auto source = std::make_shared<ArrayListModel>();
    auto model = CastListModel<int>(source);
    source->push(221);
    auto& base_model = static_cast<ListModel&>(model);
    REQUIRE(base_model.set(0, std::string("hello")) == QValidator::Invalid);
    REQUIRE(model.get(0) == 221);
    REQUIRE(base_model.set(0, 56) == QValidator::Acceptable);
    REQUIRE(model.get(0) == 56);
    model.set(0, 42);
    REQUIRE(model.get(0) == 42);
  }
}
