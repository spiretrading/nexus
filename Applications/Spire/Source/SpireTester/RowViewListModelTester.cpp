#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/RowViewListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("RowViewListModel") {
  TEST_CASE("construct") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    auto model1 = RowViewListModel(source, 2);
    REQUIRE(model1.get_size() == 0);
    auto model2 = RowViewListModel(source, -1);
    REQUIRE(model2.get_size() == 0);
    auto model3 = RowViewListModel(source, 0);
    REQUIRE(model3.get_size() == 3);
    REQUIRE(model3.get<int>(0) == 1);
    REQUIRE(model3.get<int>(1) == 2);
    REQUIRE(model3.get<int>(2) == 3);
  }

  TEST_CASE("update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    auto model = RowViewListModel(source, 1);
    REQUIRE(model.get<int>(0) == 4);
    REQUIRE(model.get<int>(1) == 5);
    REQUIRE(model.get<int>(2) == 6);
    auto signal_count = 0;
    auto updated_index = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
        auto update_operation = get<ListModel::UpdateOperation>(&operation);
        REQUIRE(update_operation != nullptr);
        REQUIRE(update_operation->m_index == updated_index);
      }));
    updated_index = 4;
    REQUIRE(model.set(updated_index, 0) == QValidator::State::Invalid);
    REQUIRE(signal_count == 0);
    updated_index = -1;
    REQUIRE(model.set(updated_index, 0) == QValidator::State::Invalid);
    REQUIRE(signal_count == 0);
    updated_index = 2;
    REQUIRE(model.set(updated_index, 0) == QValidator::State::Acceptable);
    REQUIRE(signal_count == 1);
    REQUIRE(model.get<int>(0) == 4);
    REQUIRE(model.get<int>(1) == 5);
    REQUIRE(model.get<int>(2) == 0);
  }

  TEST_CASE("source_add") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1});
    source->push({2});
    auto invalid_model = RowViewListModel(source, 3);
    REQUIRE(invalid_model.get_size() == 0);
    auto signal_count1 = 0;
    auto connection1 = scoped_connection(invalid_model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count1;
      }));
    auto model = RowViewListModel(source, 1);
    REQUIRE(model.get<int>(0) == 2);
    auto signal_count2 = 0;
    auto connection2 = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count2;
      }));
    source->push({3});
    REQUIRE(signal_count1 == 0);
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(signal_count2 == 0);
    REQUIRE(model.get<int>(0) == 2);
    source->insert({4}, 1);
    REQUIRE(signal_count1 == 0);
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(signal_count2 == 0);
    REQUIRE(model.get<int>(0) == 2);
    source->insert({5}, 0);
    REQUIRE(signal_count1 == 0);
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(signal_count2 == 0);
    REQUIRE(model.get<int>(0) == 2);
    source->push({6});
    REQUIRE(signal_count1 == 0);
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(signal_count2 == 0);
    REQUIRE(model.get<int>(0) == 2);
  }

  TEST_CASE("source_remove") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1});
    source->push({2});
    source->push({3});
    source->push({4});
    source->push({5});
    auto model = RowViewListModel(source, 2);
    REQUIRE(model.get<int>(0) == 3);
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
      }));
    source->remove(3);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get<int>(0) == 3);
    source->remove(0);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get<int>(0) == 3);
    source->remove(1);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS(model.get<int>(0));
    source->insert({6}, 2);
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS(model.get<int>(0));
  }

  TEST_CASE("source_move") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({0});
    source->push({1});
    source->push({2});
    source->push({3});
    source->push({4});
    source->push({5});
    auto model = RowViewListModel(source, 2);
    REQUIRE(model.get<int>(0) == 2);
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
      }));
    source->move(3, 5);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get<int>(0) == 2);
    source->move(2, 4);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get<int>(0) == 2);
    source->move(0, 5);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get<int>(0) == 2);
    source->move(4, 1);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get<int>(0) == 2);
    source->move(3, 0);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get<int>(0) == 2);
  }

  TEST_CASE("source_update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    auto model = RowViewListModel(source, 1);
    REQUIRE(model.get<int>(0) == 4);
    REQUIRE(model.get<int>(1) == 5);
    REQUIRE(model.get<int>(2) == 6);
    auto signal_count = 0;
    auto updated_index = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
        auto update_operation = get<ListModel::UpdateOperation>(&operation);
        REQUIRE(update_operation != nullptr);
        REQUIRE(update_operation->m_index == updated_index);
      }));
    source->set(0, 0, 0);
    REQUIRE(signal_count == 0);
    updated_index = 0;
    source->set(1, updated_index, 0);
    REQUIRE(signal_count == 1);
    REQUIRE(model.get<int>(updated_index) == 0);
    updated_index = 2;
    source->set(1, updated_index, 10);
    REQUIRE(model.get<int>(updated_index) == 10);
  }

  TEST_CASE("source_transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    auto model = RowViewListModel(source, 1);
    auto signal_count = 0;
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
        visit<ListModel>(operation,
          [&] (const ListModel::AddOperation& operation) {
            ++add_count;
          },
          [&] (const ListModel::MoveOperation& operation) {
            ++move_count;
          },
          [&] (const ListModel::RemoveOperation& operation) {
            ++remove_count;
          },
          [&] (const ListModel::UpdateOperation& operation) {
            ++update_count;
          });
      }));
    source->transact([&] {
      source->push({7, 8, 9});
      source->transact([&] {
        source->set(1, 2, 0);
        source->transact([&] {
          source->insert({10, 11, 12}, 1);
          source->set(2, 1, 0);
          source->remove(0);
        });
        source->set(1, 0, 0);
      });
      source->move(2, 0);
    });
    REQUIRE(signal_count == 1);
    REQUIRE(add_count == 0);
    REQUIRE(move_count == 0);
    REQUIRE(remove_count == 0);
    REQUIRE(update_count == 3);
    REQUIRE(model.get<int>(0) == 0);
    REQUIRE(model.get<int>(1) == 0);
    REQUIRE(model.get<int>(2) == 0);
  }
}
