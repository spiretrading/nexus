#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TransformListModel.hpp"

using namespace Spire;

TEST_SUITE("TransformListModel") {
  TEST_CASE("simplified_transformers") {
    auto source = std::make_shared<ArrayListModel<int>>();
    for(auto i = 0; i != 10; ++i) {
      source->push(i);
    }
    auto model = TransformListModel(source,
      [] (int current) {
        return std::to_string(current);
      },
      [] (const std::string& current) {
        return std::stoi(current);
      });
    REQUIRE(model.get(0) == "0");
    source->set(0, 123);
    REQUIRE(model.get(0) == "123");
    REQUIRE(model.set(0, "456") == QValidator::State::Acceptable);
    REQUIRE(source->get(0) == 456);
    REQUIRE(model.set(0, "abc") == QValidator::State::Invalid);
  }
}
