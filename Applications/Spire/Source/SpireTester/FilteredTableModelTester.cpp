#include <deque>
#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/FilteredTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("FilteredTableModel") {
  TEST_CASE("filter") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) > 3;
      });
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get_row_size() == 2);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
  }

  TEST_CASE("push") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({9});
    source->push({4});
    source->push({2});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) > 4;
      });
    REQUIRE(filtered_model.get<int>(0, 0) == 4);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(0, 0) == 9);
    REQUIRE(source->get<int>(1, 0) == 4);
    REQUIRE(source->get<int>(2, 0) == 2);
    auto signal_count = 0;
    auto connection = scoped_connection(filtered_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == filtered_model.get_row_size() - 1);
      }));
    source->push({10});
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_model.get<int>(0, 0) == 4);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(filtered_model.get_row_size() == 2);
    REQUIRE(source->get<int>(0, 0) == 9);
    REQUIRE(source->get<int>(1, 0) == 4);
    REQUIRE(source->get<int>(2, 0) == 2);
    REQUIRE(source->get<int>(3, 0) == 10);
    source->push({1});
    REQUIRE(signal_count == 1);
    REQUIRE(filtered_model.get<int>(0, 0) == 4);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(filtered_model.get<int>(2, 0) == 1);
    REQUIRE(filtered_model.get_row_size() == 3);
    REQUIRE(source->get<int>(0, 0) == 9);
    REQUIRE(source->get<int>(1, 0) == 4);
    REQUIRE(source->get<int>(2, 0) == 2);
    REQUIRE(source->get<int>(3, 0) == 10);
    REQUIRE(source->get<int>(4, 0) == 1);
  }

  TEST_CASE("insert") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1});
    source->push({4});
    source->push({2});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) > 3;
      });
    REQUIRE(filtered_model.get<int>(0, 0) == 1);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(0, 0) == 1);
    REQUIRE(source->get<int>(1, 0) == 4);
    REQUIRE(source->get<int>(2, 0) == 2);
    auto signal_count = 0;
    auto added_index = 0;
    auto connection = scoped_connection(filtered_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == added_index);
      }));
    source->insert({9}, 1);
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_model.get<int>(0, 0) == 1);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(filtered_model.get_row_size() == 2);
    REQUIRE(source->get<int>(0, 0) == 1);
    REQUIRE(source->get<int>(1, 0) == 9);
    REQUIRE(source->get<int>(2, 0) == 4);
    REQUIRE(source->get<int>(3, 0) == 2);
    added_index = 1;
    source->insert({0}, 2);
    REQUIRE(signal_count == 1);
    REQUIRE(filtered_model.get<int>(0, 0) == 1);
    REQUIRE(filtered_model.get<int>(1, 0) == 0);
    REQUIRE(filtered_model.get<int>(2, 0) == 2);
    REQUIRE(filtered_model.get_row_size() == 3);
    REQUIRE(source->get<int>(0, 0) == 1);
    REQUIRE(source->get<int>(1, 0) == 9);
    REQUIRE(source->get<int>(2, 0) == 0);
    REQUIRE(source->get<int>(3, 0) == 4);
    REQUIRE(source->get<int>(4, 0) == 2);
    added_index = 3;
    source->insert({0}, 5);
    REQUIRE(signal_count == 2);
    REQUIRE(filtered_model.get<int>(0, 0) == 1);
    REQUIRE(filtered_model.get<int>(1, 0) == 0);
    REQUIRE(filtered_model.get<int>(2, 0) == 2);
    REQUIRE(filtered_model.get<int>(3, 0) == 0);
    REQUIRE(filtered_model.get_row_size() == 4);
    REQUIRE(source->get<int>(0, 0) == 1);
    REQUIRE(source->get<int>(1, 0) == 9);
    REQUIRE(source->get<int>(2, 0) == 0);
    REQUIRE(source->get<int>(3, 0) == 4);
    REQUIRE(source->get<int>(4, 0) == 2);
    REQUIRE(source->get<int>(5, 0) == 0);
  }

  TEST_CASE("remove") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) > 3;
      });
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    auto signal_count = 0;
    auto connection = scoped_connection(filtered_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto remove_operation = get<TableModel::RemoveOperation>(&operation);
        REQUIRE(remove_operation != nullptr);
        REQUIRE(remove_operation->m_index == 0);
      }));
    source->remove(2);
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get_row_size() == 2);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 1);
    source->remove(1);
    REQUIRE(signal_count == 1);
    REQUIRE(filtered_model.get<int>(0, 0) == 1);
    REQUIRE(filtered_model.get_row_size() == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 1);
    source->remove(1);
    REQUIRE(signal_count == 2);
    REQUIRE(filtered_model.get_row_size() == 0);
    REQUIRE(source->get<int>(0, 0) == 4);
  }

  TEST_CASE("move") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({6});
    source->push({1});
    source->push({3});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) > 3;
      });
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get<int>(2, 0) == 3);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 6);
    REQUIRE(source->get<int>(4, 0) == 1);
    REQUIRE(source->get<int>(5, 0) == 3);
    auto signal_count = 0;
    auto source_row = 0;
    auto destination_row = 0;
    auto connection = scoped_connection(filtered_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto move_operation = get<TableModel::MoveOperation>(&operation);
        REQUIRE(move_operation != nullptr);
        REQUIRE(move_operation->m_source == source_row);
        REQUIRE(move_operation->m_destination == destination_row);
      }));
    source->move(3, 0);
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get<int>(2, 0) == 3);
    REQUIRE(source->get<int>(0, 0) == 6);
    REQUIRE(source->get<int>(1, 0) == 4);
    REQUIRE(source->get<int>(2, 0) == 2);
    REQUIRE(source->get<int>(3, 0) == 9);
    REQUIRE(source->get<int>(4, 0) == 1);
    REQUIRE(source->get<int>(5, 0) == 3);
    source->move(0, 4);
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get<int>(2, 0) == 3);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    REQUIRE(source->get<int>(4, 0) == 6);
    REQUIRE(source->get<int>(5, 0) == 3);
    source_row = 0;
    destination_row = 2;
    source->move(1, 5);
    REQUIRE(signal_count == 1);
    REQUIRE(filtered_model.get<int>(0, 0) == 1);
    REQUIRE(filtered_model.get<int>(1, 0) == 3);
    REQUIRE(filtered_model.get<int>(2, 0) == 2);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 9);
    REQUIRE(source->get<int>(2, 0) == 1);
    REQUIRE(source->get<int>(3, 0) == 6);
    REQUIRE(source->get<int>(4, 0) == 3);
    REQUIRE(source->get<int>(5, 0) == 2);
    source_row = 1;
    destination_row = 0;
    source->move(4, 0);
    REQUIRE(signal_count == 2);
    REQUIRE(filtered_model.get<int>(0, 0) == 3);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get<int>(2, 0) == 2);
    REQUIRE(source->get<int>(0, 0) == 3);
    REQUIRE(source->get<int>(1, 0) == 4);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    REQUIRE(source->get<int>(4, 0) == 6);
    REQUIRE(source->get<int>(5, 0) == 2);
  }

  TEST_CASE("update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) > 3;
      });
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    auto signal_count = 0;
    auto add_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto connection = scoped_connection(filtered_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const TableModel::AddOperation& add_operation) {
            ++add_count;
            REQUIRE(add_operation.m_index == 0);
          },
          [&] (const TableModel::RemoveOperation& remove_operation) {
            ++remove_count;
            REQUIRE(remove_operation.m_index == 2);
          },
          [&] (const TableModel::UpdateOperation& update_operation) {
            ++update_count;
            REQUIRE(update_operation.m_row == 1);
            REQUIRE(update_operation.m_column == 0);
          },
          [] (const auto&) {
            REQUIRE(false);
          });
      }));
    source->set(0, 0, 0);
    REQUIRE(signal_count == 1);
    REQUIRE(add_count == 1);
    REQUIRE(filtered_model.get<int>(0, 0) == 0);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(filtered_model.get<int>(2, 0) == 1);
    REQUIRE(filtered_model.get_row_size() == 3);
    REQUIRE(source->get<int>(0, 0) == 0);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    source->set(3, 0, 10);
    REQUIRE(signal_count == 2);
    REQUIRE(remove_count == 1);
    REQUIRE(filtered_model.get<int>(0, 0) == 0);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(filtered_model.get_row_size() == 2);
    REQUIRE(source->get<int>(0, 0) == 0);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 10);
    source->set(1, 0, 1);
    REQUIRE(signal_count == 3);
    REQUIRE(update_count == 1);
    REQUIRE(filtered_model.get<int>(0, 0) == 0);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get_row_size() == 2);
    REQUIRE(source->get<int>(0, 0) == 0);
    REQUIRE(source->get<int>(1, 0) == 1);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 10);
    source->set(3, 0, 6);
    REQUIRE(signal_count == 3);
    REQUIRE(filtered_model.get<int>(0, 0) == 0);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get_row_size() == 2);
    REQUIRE(source->get<int>(0, 0) == 0);
    REQUIRE(source->get<int>(1, 0) == 1);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 6);
  }

  TEST_CASE("transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) > 3;
      });
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    auto signal_count = 0;
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto connection = scoped_connection(filtered_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const TableModel::AddOperation& add_operation) {
            ++add_count;
          },
          [&] (const TableModel::MoveOperation& move_operation) {
            ++move_count;
            REQUIRE(move_operation.m_source == 2);
            REQUIRE(move_operation.m_destination == 0);
          },
          [&] (const TableModel::RemoveOperation& remove_operation) {
            ++remove_count;
            REQUIRE(remove_operation.m_index == 3);
          },
          [&] (const TableModel::UpdateOperation& update_operation) {
            ++update_count;
            REQUIRE(update_operation.m_row == 0);
            REQUIRE(update_operation.m_column == 0);
          });
      }));
    source->transact([&] {
      source->push({3});
      source->transact([&] {
        source->set(0, 0, 0);
        source->transact([&] {
          source->insert({1}, 1);
          source->remove(4);
        });
        source->set(0, 0, 1);
      });
      source->move(2, 0);
    });
    REQUIRE(signal_count == 1);
    REQUIRE(add_count == 3);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 1);
    REQUIRE(filtered_model.get<int>(0, 0) == 2);
    REQUIRE(filtered_model.get<int>(1, 0) == 1);
    REQUIRE(filtered_model.get<int>(2, 0) == 1);
    REQUIRE(filtered_model.get<int>(3, 0) == 3);
    REQUIRE(filtered_model.get_row_size() == 4);
  }

  TEST_CASE("set_filter") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({0});
    source->push({1});
    source->push({2});
    source->push({3});
    source->push({4});
    source->push({5});
    source->push({6});
    source->push({7});
    source->push({8});
    source->push({9});
    source->push({10});
    source->push({12});
    source->push({14});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) % 2 == 0;
      });
    REQUIRE(filtered_model.get_row_size() == 5);
    REQUIRE(filtered_model.get<int>(0, 0) == 1);
    REQUIRE(filtered_model.get<int>(1, 0) == 3);
    REQUIRE(filtered_model.get<int>(2, 0) == 5);
    REQUIRE(filtered_model.get<int>(3, 0) == 7);
    REQUIRE(filtered_model.get<int>(4, 0) == 9);
    auto operations = std::deque<TableModel::Operation>();
    filtered_model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    filtered_model.set_filter([] (const TableModel& model, int row) {
      return model.get<int>(row, 0) % 2 != 0;
    });
    REQUIRE(filtered_model.get_row_size() == 8);
    REQUIRE(filtered_model.get<int>(0, 0) == 0);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(filtered_model.get<int>(2, 0) == 4);
    REQUIRE(filtered_model.get<int>(3, 0) == 6);
    REQUIRE(filtered_model.get<int>(4, 0) == 8);
    REQUIRE(filtered_model.get<int>(5, 0) == 10);
    REQUIRE(filtered_model.get<int>(6, 0) == 12);
    REQUIRE(filtered_model.get<int>(7, 0) == 14);
    REQUIRE(operations.size() == 1);
    auto transaction = get<TableModel::Transaction>(&operations.front());
    REQUIRE(transaction != nullptr);
    REQUIRE(transaction->m_operations.size() == 13);
    auto add = get<TableModel::AddOperation>(&transaction->m_operations[0]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 0);
    auto remove =
      get<TableModel::RemoveOperation>(&transaction->m_operations[1]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 1);
    add = get<TableModel::AddOperation>(&transaction->m_operations[2]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 1);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[3]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 2);
    add = get<TableModel::AddOperation>(&transaction->m_operations[4]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 2);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[5]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 3);
    add = get<TableModel::AddOperation>(&transaction->m_operations[6]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 3);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[7]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 4);
    add = get<TableModel::AddOperation>(&transaction->m_operations[8]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 4);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[9]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 5);
    add = get<TableModel::AddOperation>(&transaction->m_operations[10]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 5);
    add = get<TableModel::AddOperation>(&transaction->m_operations[11]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 6);
    add = get<TableModel::AddOperation>(&transaction->m_operations[12]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 7);
  }

  TEST_CASE("set_filter_exclusive_trailing") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({0});
    source->push({1});
    source->push({2});
    source->push({3});
    source->push({4});
    source->push({5});
    source->push({6});
    source->push({7});
    source->push({8});
    source->push({9});
    source->push({11});
    source->push({13});
    source->push({15});
    auto filtered_model = FilteredTableModel(source,
      [] (const TableModel& model, int row) {
        return model.get<int>(row, 0) % 2 == 0;
      });
    REQUIRE(filtered_model.get_row_size() == 8);
    REQUIRE(filtered_model.get<int>(0, 0) == 1);
    REQUIRE(filtered_model.get<int>(1, 0) == 3);
    REQUIRE(filtered_model.get<int>(2, 0) == 5);
    REQUIRE(filtered_model.get<int>(3, 0) == 7);
    REQUIRE(filtered_model.get<int>(4, 0) == 9);
    REQUIRE(filtered_model.get<int>(5, 0) == 11);
    REQUIRE(filtered_model.get<int>(6, 0) == 13);
    REQUIRE(filtered_model.get<int>(7, 0) == 15);
    auto operations = std::deque<TableModel::Operation>();
    filtered_model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    filtered_model.set_filter([] (const TableModel& model, int row) {
      return model.get<int>(row, 0) % 2 != 0;
    });
    REQUIRE(filtered_model.get_row_size() == 5);
    REQUIRE(filtered_model.get<int>(0, 0) == 0);
    REQUIRE(filtered_model.get<int>(1, 0) == 2);
    REQUIRE(filtered_model.get<int>(2, 0) == 4);
    REQUIRE(filtered_model.get<int>(3, 0) == 6);
    REQUIRE(filtered_model.get<int>(4, 0) == 8);
    REQUIRE(operations.size() == 1);
    auto transaction = get<TableModel::Transaction>(&operations.front());
    REQUIRE(transaction != nullptr);
    REQUIRE(transaction->m_operations.size() == 13);
    auto add = get<TableModel::AddOperation>(&transaction->m_operations[0]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 0);
    auto remove =
      get<TableModel::RemoveOperation>(&transaction->m_operations[1]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 1);
    add = get<TableModel::AddOperation>(&transaction->m_operations[2]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 1);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[3]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 2);
    add = get<TableModel::AddOperation>(&transaction->m_operations[4]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 2);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[5]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 3);
    add = get<TableModel::AddOperation>(&transaction->m_operations[6]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 3);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[7]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 4);
    add = get<TableModel::AddOperation>(&transaction->m_operations[8]);
    REQUIRE(add != nullptr);
    REQUIRE(add->m_index == 4);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[9]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 5);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[10]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 5);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[11]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 5);
    remove = get<TableModel::RemoveOperation>(&transaction->m_operations[12]);
    REQUIRE(remove != nullptr);
    REQUIRE(remove->m_index == 5);
  }
}
