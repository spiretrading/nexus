#include <doctest/doctest.h>
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/ListValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  int get_current(const ListValueModel& model) {
    return std::any_cast<int>(model.get_current());
  }
}

TEST_SUITE("ListValueModel") {
  TEST_CASE("construct") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    auto model1 = ListValueModel(source, 0);
    REQUIRE(model1.get_state() == QValidator::State::Acceptable);
    REQUIRE(get_current(model1) == 0);
    auto model2 = ListValueModel(source, -1);
    REQUIRE(model2.get_state() == QValidator::State::Invalid);
    REQUIRE(!model2.get_current().has_value());
    auto model3 = ListValueModel(source, 2);
    REQUIRE(model3.get_state() == QValidator::State::Invalid);
    REQUIRE(!model3.get_current().has_value());
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
    REQUIRE(get_current(model1) == value);
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
    REQUIRE(!model2.get_current().has_value());
  }

  TEST_CASE("source_add1") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    auto signal_count = 0;
    auto index = 1;
    auto model = ListValueModel(source, index);
    REQUIRE(get_current(model) == 1);
    auto connection = scoped_connection(model.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count;
        REQUIRE(std::any_cast<int>(current) == source->get<int>(index));
      }));
    source->insert(10, 1);
    REQUIRE(signal_count == 1);
    REQUIRE(get_current(model) == 10);
    source->insert(20, 0);
    REQUIRE(signal_count == 2);
    REQUIRE(get_current(model) == 0);
    source->push(30);
    REQUIRE(signal_count == 2);
    REQUIRE(get_current(model) == 0);
  }

  TEST_CASE("source_add2") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    auto signal_count = 0;
    auto index = 3;
    auto model = ListValueModel(source, index);
    REQUIRE(model.get_state() == QValidator::State::Invalid);
    auto connection = scoped_connection(model.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count;
        REQUIRE(std::any_cast<int>(current) == source->get<int>(index));
      }));
    source->insert(10, 0);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get_state() == QValidator::State::Invalid);
    REQUIRE(!model.get_current().has_value());
    source->insert(20, 0);
    REQUIRE(signal_count == 1);
    REQUIRE(model.get_state() == QValidator::State::Acceptable);
    REQUIRE(get_current(model) == 1);
  }

  TEST_CASE("source_remove") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    source->push(2);
    source->push(3);
    auto signal_count = 0;
    auto index = 1;
    auto model = ListValueModel(source, index);
    REQUIRE(get_current(model) == 1);
    auto connection = scoped_connection(model.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count;
        if(index < source->get_size()) {
          REQUIRE(std::any_cast<int>(current) == source->get<int>(index));
        } else {
          REQUIRE(!current.has_value());
        }
      }));
    source->remove(3);
    REQUIRE(signal_count == 0);
    REQUIRE(get_current(model) == 1);
    source->remove(1);
    REQUIRE(signal_count == 1);
    REQUIRE(get_current(model) == 2);
    source->remove(0);
    REQUIRE(signal_count == 2);
    REQUIRE(!model.get_current().has_value());
    source->remove(0);
    REQUIRE(signal_count == 2);
    REQUIRE(!model.get_current().has_value());
  }

  TEST_CASE("source_move") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    source->push(2);
    source->push(3);
    auto signal_count = 0;
    auto index = 1;
    auto model = ListValueModel(source, index);
    REQUIRE(get_current(model) == 1);
    auto connection = scoped_connection(model.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count;
        REQUIRE(std::any_cast<int>(current) == source->get<int>(index));
      }));
    source->move(2, 3);
    REQUIRE(signal_count == 0);
    REQUIRE(get_current(model) == 1);
    source->move(0, 2);
    REQUIRE(signal_count == 1);
    REQUIRE(get_current(model) == 3);
    source->move(3, 1);
    REQUIRE(signal_count == 2);
    REQUIRE(get_current(model) == 2);
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
    REQUIRE(get_current(model) == 10);
    source->set(1, 20);
    REQUIRE(signal_count == 1);
    REQUIRE(get_current(model) == 10);
  }

  TEST_CASE("source_transaction") {
    auto source = std::make_shared<ArrayListModel>();
    source->push(0);
    source->push(1);
    auto signal_count = 0;
    auto index = 1;
    auto model = ListValueModel(source, index);
    auto connection = scoped_connection(model.connect_current_signal(
      [&] (const auto& current) {
        ++signal_count;
        REQUIRE(std::any_cast<int>(current) == source->get<int>(index));
      }));
    source->transact([&] {
      source->push(2);
      source->transact([&] {
        source->set(1, 10);
        source->transact([&] {
          source->insert(20, 0);
          source->set(2, 30);
          source->remove(0);
        });
        source->set(0, 10);
      });
      source->move(1, 0);
    });
    REQUIRE(signal_count == 4);
    REQUIRE(get_current(model) == 10);
  }
}
