#include <deque>
#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/ColumnViewListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  template<typename... F>
  decltype(auto) test_operation(
      const ListModel::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }
}

TEST_SUITE("ColumnViewListModel") {
  TEST_CASE("construct") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    source->push({7, 8, 9});
    auto model1 = ColumnViewListModel(source, 4);
    REQUIRE(model1.get_size() == 0);
    auto model2 = ColumnViewListModel(source, -1);
    REQUIRE(model2.get_size() == 0);
    auto model3 = ColumnViewListModel(source, 1);
    REQUIRE(model3.get_size() == 3);
    REQUIRE(model3.get<int>(0) == 2);
    REQUIRE(model3.get<int>(1) == 5);
    REQUIRE(model3.get<int>(2) == 8);
  }

  TEST_CASE("update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    source->push({7, 8, 9});
    auto model = ColumnViewListModel(source, 1);
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 5);
    REQUIRE(model.get<int>(2) == 8);
    auto operations = std::deque<ListModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    REQUIRE(model.set(4, 0) == QValidator::State::Invalid);
    REQUIRE(operations.empty());
    REQUIRE(model.set(-1, 0) == QValidator::State::Invalid);
    REQUIRE(operations.empty());
    REQUIRE(model.set(2, 0) == QValidator::State::Acceptable);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 5);
    REQUIRE(model.get<int>(2) == 0);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation,
      [&] (const ListModel::UpdateOperation& operation) {
        REQUIRE(operation.m_index == 2);
      });
  }

  TEST_CASE("source_add") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2});
    source->push({3, 4});
    auto invalid_model = ColumnViewListModel(source, 2);
    REQUIRE(invalid_model.get_size() == 0);
    auto operations1 = std::deque<ListModel::Operation>();
    auto connection1 = scoped_connection(invalid_model.connect_operation_signal(
      [&] (const auto& operation) {
        operations1.push_back(operation);
      }));
    auto model = ColumnViewListModel(source, 1);
    REQUIRE(model.get_size() == 2);
    auto operations2 = std::deque<ListModel::Operation>();
    auto connection2 = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations2.push_back(operation);
      }));
    source->push({5, 6});
    REQUIRE(operations1.empty());
    REQUIRE(operations2.size() == 1);
    auto operation = operations2.front();
    operations2.pop_front();
    test_operation(operation, [&] (const ListModel::AddOperation& operation) {
      REQUIRE(operation.m_index == 2);
    });
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 4);
    REQUIRE(model.get<int>(2) == 6);
    source->insert({7, 8}, 1);
    REQUIRE(operations1.empty());
    REQUIRE(operations2.size() == 1);
    operation = operations2.front();
    operations2.pop_front();
    test_operation(operation, [&] (const ListModel::AddOperation& operation) {
      REQUIRE(operation.m_index == 1);
    });
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(model.get_size() == 4);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 8);
    REQUIRE(model.get<int>(2) == 4);
    REQUIRE(model.get<int>(3) == 6);
  }

  TEST_CASE("source_remove") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2});
    source->push({3, 4});
    source->push({5, 6});
    source->push({7, 8});
    auto model = ColumnViewListModel(source, 1);
    REQUIRE(model.get_size() == 4);
    auto index = 0;
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
        auto remove_operation = get<ListModel::RemoveOperation>(&operation);
        REQUIRE(remove_operation != nullptr);
        REQUIRE(remove_operation->m_index == index);
      }));
    index = 3;
    source->remove(index);
    REQUIRE(signal_count == 1);
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 4);
    REQUIRE(model.get<int>(2) == 6);
    index = 1;
    source->remove(index);
    REQUIRE(signal_count == 2);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 6);
  }

  TEST_CASE("source_move") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({0, 1});
    source->push({2, 3});
    source->push({4, 5});
    source->push({6, 7});
    auto model = ColumnViewListModel(source, 1);
    REQUIRE(model.get_size() == 4);
    auto signal_count = 0;
    auto source_index = 0;
    auto destination_index = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
        auto move_operation = get<ListModel::MoveOperation>(&operation);
        REQUIRE(move_operation != nullptr);
        REQUIRE(move_operation->m_source == source_index);
        REQUIRE(move_operation->m_destination == destination_index);
      }));
    source_index = 1;
    destination_index = 3;
    source->move(source_index, destination_index);
    REQUIRE(signal_count == 1);
    REQUIRE(model.get<int>(0) == 1);
    REQUIRE(model.get<int>(1) == 5);
    REQUIRE(model.get<int>(2) == 7);
    REQUIRE(model.get<int>(3) == 3);
    source_index = 3;
    destination_index = 0;
    source->move(source_index, destination_index);
    REQUIRE(signal_count == 2);
    REQUIRE(model.get<int>(0) == 3);
    REQUIRE(model.get<int>(1) == 1);
    REQUIRE(model.get<int>(2) == 5);
    REQUIRE(model.get<int>(3) == 7);
  }

  TEST_CASE("source_update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2});
    source->push({3, 4});
    source->push({5, 6});
    auto model = ColumnViewListModel(source, 1);
    REQUIRE(model.get_size() == 3);
    auto operations = std::deque<ListModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->set(0, 0, 0);
    REQUIRE(operations.empty());
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 4);
    REQUIRE(model.get<int>(2) == 6);
    source->set(2, 1, 0);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 4);
    REQUIRE(model.get<int>(2) == 0);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation,
      [&] (const ListModel::UpdateOperation& operation) {
        REQUIRE(operation.m_index == 2);
      });
  }

  TEST_CASE("source_transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2});
    source->push({3, 4});
    source->push({5, 6});
    auto model = ColumnViewListModel(source, 1);
    REQUIRE(model.get_size() == 3);
    auto operations = std::deque<ListModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->transact([&] {
      source->push({7, 8});
      source->transact([&] {
        source->set(0, 1, 0);
        source->transact([&] {
          source->remove(1);
          source->insert({9, 10}, 2);
        });
        source->move(2, 0);
      });
    });
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    test_operation(operation,
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
    REQUIRE(add_count == 2);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 1);
    REQUIRE(model.get<int>(0) == 10);
    REQUIRE(model.get<int>(1) == 0);
    REQUIRE(model.get<int>(2) == 6);
    REQUIRE(model.get<int>(3) == 8);
  }
}
