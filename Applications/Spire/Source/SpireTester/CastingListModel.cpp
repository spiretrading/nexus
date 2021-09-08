#include <doctest/doctest.h>
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/CastingListModel.hpp"

using namespace Spire;

TEST_SUITE("CastingListModel") {
  TEST_CASE("get_set") {
    auto source = std::make_shared<ArrayListModel>();
    auto int_model = std::make_shared<CastingListModel<int>>(source);
    source->push(123);
    REQUIRE(int_model->get(0) == 123);
    REQUIRE(int_model->set(0, 321) == QValidator::Acceptable);
    REQUIRE(source->get<int>(0) == 321);
  }
}
