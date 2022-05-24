#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/TranslatedTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  template<typename... F>
  decltype(auto) test_operation(
      const TableModel::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }
}

TEST_SUITE("TranslatedTableModel") {
  TEST_CASE("translate") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto translation = TranslatedTableModel(source);
    auto operations = std::deque<TableModel::Operation>();
    translation.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const TableModel::MoveOperation& operation) {
      REQUIRE(operation.m_source == 3);
      REQUIRE(operation.m_destination == 0);
    });
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 4);
    REQUIRE(translation.get<int>(2, 0) == 2);
    REQUIRE(translation.get<int>(3, 0) == 9);
    REQUIRE_THROWS(translation.move(6, 2));
    REQUIRE(operations.empty());
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const TableModel::MoveOperation& operation) {
      REQUIRE(operation.m_source == 1);
      REQUIRE(operation.m_destination == 2);
    });
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
  }

  TEST_CASE("transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({10});
    source->push({9});
    source->push({1});
    source->push({6});
    auto translation = TranslatedTableModel(source);
    auto operations = std::deque<TableModel::Operation>();
    translation.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
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
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    auto move_count = 0;
    test_operation(operation, [&] (const TableModel::MoveOperation& operation) {
      ++move_count;
    });
    REQUIRE(move_count == 4);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 6);
    REQUIRE(translation.get<int>(4, 0) == 9);
    REQUIRE(translation.get<int>(5, 0) == 10);
  }

  TEST_CASE("transaction_with_one_operation") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({1});
    source->push({6});
    auto signal_count = 0;
    auto translation = TranslatedTableModel(source);
    auto operations = std::deque<TableModel::Operation>();
    translation.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    translation.transact([&] {
      translation.move(0, 2);
      translation.transact([] {});
    });
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const TableModel::MoveOperation& operation) {
      REQUIRE(operation.m_source == 0);
      REQUIRE(operation.m_destination == 2);
    });
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 6);
    REQUIRE(translation.get<int>(2, 0) == 4);
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
        auto transaction = operation.get<TableModel::Transaction>();
        REQUIRE((transaction != none));
        REQUIRE(transaction->size() == 3);
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
        auto transaction = operation.get<TableModel::Transaction>();
        REQUIRE((transaction != none));
        REQUIRE(transaction->size() == 4);
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
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
        REQUIRE(add_operation->m_index == translation.get_row_size() - 1);
      }));
    source->push({6});
    REQUIRE(signal_count == 1);
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
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
        REQUIRE(add_operation->m_index == 3);
      }));
    source->insert({6}, 2);
    REQUIRE(signal_count == 1);
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
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    source->move(2, 0);
    REQUIRE(signal_count == 0);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
  }

  TEST_CASE("trivial_move") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({std::string("A")});
    source->push({std::string("B")});
    source->push({std::string("C")});
    auto translation = TranslatedTableModel(source);
    auto operations = std::deque<TableModel::Operation>();
    translation.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    source->move(0, 2);
    REQUIRE(operations.size() == 0);
    source->set(2, 0, std::string("D"));
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    visit(operation,
      [&] (const TableModel::UpdateOperation& operation) {
        REQUIRE(operation.m_row == 0);
        REQUIRE(operation.m_column == 0);
      },
      [] (const auto&) {
        REQUIRE(false);
      });
    REQUIRE(translation.get<std::string>(0, 0) == "D");
    REQUIRE(translation.get<std::string>(1, 0) == "B");
    REQUIRE(translation.get<std::string>(2, 0) == "C");
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
        auto remove_operation = operation.get<TableModel::RemoveOperation>();
        REQUIRE((remove_operation != none));
        REQUIRE(remove_operation->m_index == removed_row);
      }));
    removed_row = 2;
    source->remove(0);
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 9);
    REQUIRE(translation.get_row_size() == 3);
    REQUIRE_THROWS(translation.get<int>(3, 0));
    removed_row = 0;
    source->remove(2);
    REQUIRE(signal_count == 2);
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
        auto update_operation = operation.get<TableModel::UpdateOperation>();
        REQUIRE((update_operation != none));
        REQUIRE(update_operation->m_row == updated_row);
        REQUIRE(update_operation->m_column == 0);
      }));
    REQUIRE(translation.get<int>(0, 0) == 1);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    updated_row = 0;
    source->set(3, 0, 10);
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 10);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 4);
    REQUIRE(translation.get<int>(3, 0) == 9);
    updated_row = 2;
    source->set(0, 0, 0);
    REQUIRE(signal_count == 2);
    REQUIRE(translation.get<int>(0, 0) == 10);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE(translation.get<int>(2, 0) == 0);
    REQUIRE(translation.get<int>(3, 0) == 9);
  }

  TEST_CASE("set_translation") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({2});
    source->push({1});
    auto translation = TranslatedTableModel(source);
    translation.move(1, 0);
    translation.set(0, 0, 10);
    REQUIRE(source->get<int>(0, 0) == 2);
    REQUIRE(source->get<int>(1, 0) == 10);
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
    REQUIRE(translation.get<int>(0, 0) == 4);
    source->push({2});
    REQUIRE(signal_count == 2);
    REQUIRE(add_count == 2);
    REQUIRE(translation.get_row_size() == 2);
    REQUIRE(translation.get<int>(0, 0) == 4);
    REQUIRE(translation.get<int>(1, 0) == 2);
    REQUIRE_NOTHROW(translation.move(1, 0));
    REQUIRE(signal_count == 3);
    REQUIRE(move_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 2);
    REQUIRE(translation.get<int>(1, 0) == 4);
    source->set(1, 0, 0);
    REQUIRE(signal_count == 4);
    REQUIRE(update_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 0);
    REQUIRE(translation.get<int>(1, 0) == 4);
    source->remove(0);
    REQUIRE(signal_count == 5);
    REQUIRE(remove_count == 1);
    REQUIRE(translation.get<int>(0, 0) == 0);
    REQUIRE(translation.get_row_size() == 1);
  }
}
