#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/TranslatedTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("TranslatedTableModel") {
  TEST_CASE("translate") {
    auto source = std::make_shared<ArrayTableModel>();
    auto empty_translation = TranslatedTableModel(source);
    REQUIRE(empty_translation.get_row_size() == 0);
    REQUIRE(empty_translation.get_column_size() == 0);
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto signal_count = 0;
    auto source_row = 0;
    auto destination_row = 0;
    auto translation = TranslatedTableModel(source);
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto move_operation = get<TableModel::MoveOperation>(&operation);
        REQUIRE(move_operation != nullptr);
        REQUIRE(move_operation->m_source == source_row);
        REQUIRE(move_operation->m_destination == destination_row);
      }));
    source_row = 3;
    destination_row = 0;
    REQUIRE_NOTHROW(translation.move(source_row, destination_row));
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 4);
    REQUIRE(translation.get<int>(2, 0) == 2);
    REQUIRE(translation.get<int>(3, 0) == 9);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    source_row = 0;
    destination_row = 6;
    REQUIRE_THROWS(translation.move(source_row, destination_row));
    REQUIRE(signal_count == 1);
    source_row = 1;
    destination_row = 2;
    REQUIRE_NOTHROW(translation.move(source_row, destination_row));
    REQUIRE(signal_count == 2);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
  }

  TEST_CASE("transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({10});
    source->push({9});
    source->push({1});
    source->push({6});
    auto signal_count = 0;
    auto translation = TranslatedTableModel(source);
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto transaction = get<TableModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 4);
      }));
    translation.transact([&] {
      translation.move(4, 0);
      translation.transact([&] {
        translation.move(1, 2);
        translation.transact([&] {
          translation.move(3, 5);
        });
        translation.move(4, 3);
      });
    });
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 6);
    REQUIRE(translation.get<int>(4, 0) == 9);
    REQUIRE(translation.get<int>(5, 0) == 10);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 10);
    REQUIRE(source->get<int>(3, 0) == 9);
    REQUIRE(source->get<int>(4, 0) == 1);
    REQUIRE(source->get<int>(5, 0) == 6);
  }

  TEST_CASE("transaction_with_one_operation") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({1});
    source->push({6});
    auto signal_count = 0;
    auto translation = TranslatedTableModel(source);
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto move_operation = get<TableModel::MoveOperation>(&operation);
        REQUIRE(move_operation != nullptr);
        REQUIRE(move_operation->m_source == 0);
        REQUIRE(move_operation->m_destination == 2);
      }));
    translation.transact([&] {
      translation.move(0, 2);
      translation.transact([&] {});
    });
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 6);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 1);
    REQUIRE(source->get<int>(2, 0) == 6);
  }

  TEST_CASE("transaction_reentrant") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({10});
    source->push({9});
    source->push({1});
    source->push({6});
    auto signal_count = 0;
    auto translation = TranslatedTableModel(source);
    connection connection = translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        connection.disconnect();
        translation.transact([&] {
          translation.move(4, 5);
        });
        auto transaction = get<TableModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 3);
      });
    translation.transact([&] {
      translation.move(4, 0);
      translation.transact([&] {
        translation.move(2, 1);
        translation.transact([&] {
          translation.move(5, 3);
        });
      });
    });
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 6);
    REQUIRE(translation.get<int>(4, 0) == 9);
    REQUIRE(translation.get<int>(5, 0) == 10);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 10);
    REQUIRE(source->get<int>(3, 0) == 9);
    REQUIRE(source->get<int>(4, 0) == 1);
    REQUIRE(source->get<int>(5, 0) == 6);
  }

  TEST_CASE("transaction_mixing_with_source_operation") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({1});
    auto signal_count = 0;
    auto translation = TranslatedTableModel(source);
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto transaction = get<TableModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 4);
      }));
    translation.transact([&] {
      source->insert({3}, 2);
      translation.move(2, 0);
      translation.transact([&] {
        source->set(2, 0, 9);
        translation.transact([&] {
          translation.move(1, 2);
        });
        source->move(3, 1);
      });
    });
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 9);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 1);
    REQUIRE(source->get<int>(2, 0) == 2);
    REQUIRE(source->get<int>(3, 0) == 9);
  }

  TEST_CASE("push_from_source") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto translation = TranslatedTableModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    auto signal_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == translation.get_row_size() - 1);
      }));
    source->push({6});
    REQUIRE(signal_count == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    REQUIRE(source->get<int>(4, 0) == 6);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    REQUIRE(translation.get<int>(4, 0) == 6);
    REQUIRE(translation.get_row_size() == 5);
  }

  TEST_CASE("insert_from_source") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto translation = TranslatedTableModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    auto signal_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == 3);
      }));
    source->insert({6}, 2);
    REQUIRE(signal_count == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 6);
    REQUIRE(source->get<int>(3, 0) == 9);
    REQUIRE(source->get<int>(4, 0) == 1);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 6);
    REQUIRE(translation.get<int>(4, 0) == 9);
    REQUIRE(translation.get_row_size() == 5);
  }

  TEST_CASE("move_from_source") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto translation = TranslatedTableModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    auto signal_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
      }));
    source->move(1, 3);
    REQUIRE(signal_count == 0);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 9);
    REQUIRE(source->get<int>(2, 0) == 1);
    REQUIRE(source->get<int>(3, 0) == 2);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    source->move(2, 0);
    REQUIRE(signal_count == 0);
    REQUIRE(source->get<int>(0, 0) == 1);
    REQUIRE(source->get<int>(1, 0) == 4);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 2);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
  }

  TEST_CASE("remove_from_source") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto translation = TranslatedTableModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    auto signal_count = 0;
    auto removed_row = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto remove_operation = get<TableModel::RemoveOperation>(&operation);
        REQUIRE(remove_operation != nullptr);
        REQUIRE(remove_operation->m_index == removed_row);
      }));
    removed_row = 2;
    source->remove(0);
    REQUIRE(signal_count == 1);
    REQUIRE(source->get<int>(0, 0) == 2);
    REQUIRE(source->get<int>(1, 0) == 9);
    REQUIRE(source->get<int>(2, 0) == 1);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 9);
    REQUIRE(translation.get_row_size() == 3);
    REQUIRE_THROWS(translation.get<int>(3, 0));
    removed_row = 0;
    source->remove(2);
    REQUIRE(signal_count == 2);
    REQUIRE(source->get<int>(0, 0) == 2);
    REQUIRE(source->get<int>(1, 0) == 9);
    REQUIRE(translation.get<int>(0, 0) == 2);
    REQUIRE(translation.get<int>(1, 0) == 9);
    REQUIRE(translation.get_row_size() == 2);
    REQUIRE_THROWS(translation.get<int>(2, 0));
  }

  TEST_CASE("update_from_source") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto translation = TranslatedTableModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(2, 1));
    auto signal_count = 0;
    auto updated_row = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto update_operation = get<TableModel::UpdateOperation>(&operation);
        REQUIRE(update_operation != nullptr);
        REQUIRE(update_operation->m_row == updated_row);
        REQUIRE(update_operation->m_column == 0);
      }));
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    updated_row = 0;
    source->set(3, 0, 10);
    REQUIRE(signal_count == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 10);
    REQUIRE(translation.get<int>(0, 0) == 10);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    updated_row = 2;
    source->set(0, 0, 0);
    REQUIRE(signal_count == 2);
    REQUIRE(source->get<int>(0, 0) == 0);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 10);
    REQUIRE(translation.get<int>(0, 0) == 10);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 0);
    REQUIRE(translation.get<int>(3, 0) == 9);
  }

  TEST_CASE("transaction_from_source") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({2});
    source->push({1});
    auto translation = TranslatedTableModel(source);
    REQUIRE_NOTHROW(translation.move(1, 0));
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    auto signal_count = 0;
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const TableModel::AddOperation& add_operation) {
            ++add_count;
          },
          [&] (const TableModel::MoveOperation& move_operation) {
            ++move_count;
          },
          [&] (const TableModel::RemoveOperation& remove_operation) {
            ++remove_count;
            REQUIRE(remove_operation.m_index == 1);
          },
          [&] (const TableModel::UpdateOperation& update_operation) {
            ++update_count;
            REQUIRE(update_operation.m_row == 1);
            REQUIRE(update_operation.m_column == 0);
          });
      }));
    source->transact([&] {
      source->push({4});
      source->transact([&] {
        source->set(0, 0, 10);
        source->transact([&] {
          source->insert({6}, 1);
          source->remove(2);
        });
        source->push({0});
      });
      source->move(3, 0);
    });
    REQUIRE(signal_count == 1);
    REQUIRE(add_count == 3);
    REQUIRE(move_count == 0);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 1);
    REQUIRE(source->get<int>(0, 0) == 0);
    REQUIRE(source->get<int>(1, 0) == 10);
    REQUIRE(source->get<int>(2, 0) == 6);
    REQUIRE(source->get<int>(3, 0) == 4);
    REQUIRE(source->get_row_size() == 4);
    REQUIRE(translation.get<int>(0, 0) == 6);
    REQUIRE(translation.get<int>(1, 0) == 10);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 0);
    REQUIRE(translation.get_row_size() == 4);
  }

  TEST_CASE("translate_mixing_with_source_operation") {
    auto source = std::make_shared<ArrayTableModel>();
    auto translation = TranslatedTableModel(source);
    REQUIRE(translation.get_row_size() == 0);
    REQUIRE(translation.get_column_size() == 0);
    auto signal_count = 0;
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const TableModel::AddOperation& add_operation) {
            ++add_count;
          },
          [&] (const TableModel::MoveOperation& move_operation) {
            ++move_count;
            REQUIRE(move_operation.m_source == 1);
            REQUIRE(move_operation.m_destination == 0);
          },
          [&] (const TableModel::RemoveOperation& remove_operation) {
            ++remove_count;
            REQUIRE(remove_operation.m_index == 1);
          },
          [&] (const TableModel::UpdateOperation& update_operation) {
            ++update_count;
            REQUIRE(update_operation.m_row == 0);
            REQUIRE(update_operation.m_column == 0);
          });
      }));
    source->push({4});
    REQUIRE(signal_count == 1);
    REQUIRE(add_count == 1);
    REQUIRE(translation.get_row_size() == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(translation.get<int>(0, 0) == 4);
    source->push({2});
    REQUIRE(signal_count == 2);
    REQUIRE(add_count == 2);
    REQUIRE(translation.get_row_size() == 2);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(0, 0) == 4);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE_NOTHROW(translation.move(1, 0));
    REQUIRE(signal_count == 3);
    REQUIRE(move_count == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(0, 0) == 2);
    REQUIRE(translation.get<int>(1, 0) == 4);
    source->set(1, 0, 0);
    REQUIRE(signal_count == 4);
    REQUIRE(update_count == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 0);
    REQUIRE(translation.get<int>(0, 0) == 0);
    REQUIRE(translation.get<int>(1, 0) == 4);
    source->remove(0);
    REQUIRE(signal_count == 5);
    REQUIRE(remove_count == 1);
    REQUIRE(source->get<int>(0, 0) == 0);
    REQUIRE(translation.get<int>(0, 0) == 0);
    REQUIRE(translation.get_row_size() == 1);
  }
}
