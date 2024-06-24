#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TransformListModel.hpp"

using namespace Spire;

TEST_SUITE("TransformListModel") {
  TEST_CASE("set") {
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
    REQUIRE(model.get(6) == "6");
    source->set(3, 123);
    REQUIRE(model.get(3) == "123");
    REQUIRE(model.set(6, "456") == QValidator::State::Acceptable);
    REQUIRE(source->get(6) == 456);
    REQUIRE(model.set(7, "abc") == QValidator::State::Invalid);
  }

  TEST_CASE("insert") {
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
    REQUIRE(model.insert(std::string("456"), 3) ==
      QValidator::State::Acceptable);
    REQUIRE(source->get(3) == 456);
  }
}
