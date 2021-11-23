#include <doctest/doctest.h>
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/ListValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  int get(const ListValueModel& model) {
    return std::any_cast<int>(model.get());
  }
}

TEST_SUITE("ListValueModel") {
  TEST_CASE("construct") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    auto model1 = ListValueModel(source, 0);
    REQUIRE(model1.get_state() == QValidator::State::Acceptable);
    REQUIRE(get(model1) == 0);
    auto model2 = ListValueModel(source, -1);
    REQUIRE(model2.get_state() == QValidator::State::Invalid);
    REQUIRE(!model2.get().has_value());
    auto model3 = ListValueModel(source, 2);
    REQUIRE(model3.get_state() == QValidator::State::Invalid);
    REQUIRE(!model3.get().has_value());
  }

  TEST_CASE("set_current") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    auto signal_count = 0;
    auto index = 0;
    auto model1 = ListValueModel(source, index);
    auto connection = scoped_connection(model1.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count;
        REQUIRE(std::any_cast<int>(current) == source->get<int>(index));
      }));
    auto value = 10;
    REQUIRE(model1.set_current(value) == QValidator::State::Acceptable);
    REQUIRE(signal_count == 1);
    REQUIRE(get(model1) == value);
    signal_count = 0;
    index = 10;
    auto model2 = ListValueModel(source, index);
    REQUIRE(model2.get_state() == QValidator::State::Invalid);
    connection = scoped_connection(model2.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count;
      }));
    REQUIRE(model2.set_current(value) == QValidator::State::Invalid);
    REQUIRE(signal_count == 0);
    REQUIRE(!model2.get().has_value());
  }

  TEST_CASE("source_add") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    auto model0 = ListValueModel(source, 0);
    REQUIRE(get(model0) == 0);
    auto model1 = ListValueModel(source, 1);
    REQUIRE(get(model1) == 1);
    source->insert(10, 1);
    auto new_model1 = ListValueModel(source, 1);
    REQUIRE(get(model0) == 0);
    REQUIRE(get(new_model1) == 10);
    REQUIRE(get(model1) == 1);
    source->insert(20, 0);
    auto new_model0 = ListValueModel(source, 0);
    REQUIRE(get(new_model0) == 20);
    REQUIRE(get(model0) == 0);
    REQUIRE(get(new_model1) == 10);
    REQUIRE(get(model1) == 1);
    source->push(30);
    REQUIRE(get(new_model0) == 20);
    REQUIRE(get(model0) == 0);
    REQUIRE(get(new_model1) == 10);
    REQUIRE(get(model1) == 1);
  }

  TEST_CASE("source_remove") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    source->push(2);
    source->push(3);
    auto model0 = ListValueModel(source, 0);
    auto model1 = ListValueModel(source, 1);
    auto model2 = ListValueModel(source, 2);
    auto model3 = ListValueModel(source, 3);
    source->remove(1);
    REQUIRE(get(model0) == 0);
    REQUIRE(get(model2) == 2);
    REQUIRE(get(model3) == 3);
    REQUIRE(model1.get_state() == QValidator::State::Invalid);
    source->remove(2);
    REQUIRE(get(model0) == 0);
    REQUIRE(get(model2) == 2);
    REQUIRE(model1.get_state() == QValidator::State::Invalid);
    REQUIRE(model3.get_state() == QValidator::State::Invalid);
    source->remove(0);
    REQUIRE(get(model2) == 2);
    REQUIRE(model0.get_state() == QValidator::State::Invalid);
    REQUIRE(model1.get_state() == QValidator::State::Invalid);
    REQUIRE(model3.get_state() == QValidator::State::Invalid);
  }

  TEST_CASE("source_move") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    source->push(2);
    source->push(3);
    auto model0 = ListValueModel(source, 0);
    auto model1 = ListValueModel(source, 1);
    auto model2 = ListValueModel(source, 2);
    auto model3 = ListValueModel(source, 3);
    source->move(0, 3);
    REQUIRE(get(model0) == 0);
    REQUIRE(get(model1) == 1);
    REQUIRE(get(model2) == 2);
    REQUIRE(get(model3) == 3);
    source->move(3, 1);
    REQUIRE(get(model0) == 0);
    REQUIRE(get(model1) == 1);
    REQUIRE(get(model2) == 2);
    REQUIRE(get(model3) == 3);
    source->move(1, 2);
    REQUIRE(get(model0) == 0);
    REQUIRE(get(model1) == 1);
    REQUIRE(get(model2) == 2);
    REQUIRE(get(model3) == 3);
  }

  TEST_CASE("source_update") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    auto signal_count = 0;
    auto index = 0;
    auto model = ListValueModel(source, index);
    auto connection = scoped_connection(model.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count;
        REQUIRE(std::any_cast<int>(current) == source->get<int>(index));
      }));
    source->set(index, 10);
    REQUIRE(signal_count == 1);
    REQUIRE(get(model) == 10);
    source->set(1, 20);
    REQUIRE(signal_count == 1);
    REQUIRE(get(model) == 10);
  }

  TEST_CASE("source_transaction") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    source->push(2);
    auto model0 = ListValueModel(source, 0);
    auto model1 = ListValueModel(source, 1);
    auto model2 = ListValueModel(source, 2);
    auto signal_count0 = 0;
    auto connection0 = scoped_connection(model0.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count0;
      }));
    auto signal_count1 = 0;
    auto connection1 = scoped_connection(model1.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count1;
      }));
    source->transact([&] {
      source->insert(20, 0);
      source->transact([&] {
        source->set(1, 10);
        source->transact([&] {
          source->move(3, 1);
        });
        source->remove(1);
      });
      source->set(2, 30);
    });
    REQUIRE(signal_count0 == 1);
    REQUIRE(signal_count1 == 1);
    REQUIRE(get(model0) == 10);
    REQUIRE(get(model1) == 30);
    REQUIRE(model2.get_state() == QValidator::State::Invalid);
  }
}
