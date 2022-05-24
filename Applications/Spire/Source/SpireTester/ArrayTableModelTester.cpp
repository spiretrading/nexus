#include <boost/variant/get.hpp>
#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ArrayTableModel") {
  TEST_CASE("push") {
    auto model = ArrayTableModel();
    REQUIRE(model.get_row_size() == 0);
    REQUIRE(model.get_column_size() == 0);
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
      }));
    REQUIRE_NOTHROW(model.push({3, 1, 4}));
    REQUIRE(signal_count == 1);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(model.get<int>(0, 0) == 3);
    REQUIRE(model.get<int>(0, 1) == 1);
    REQUIRE(model.get<int>(0, 2) == 4);
    REQUIRE_THROWS(model.push({2, 7}));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push({2, 7, 1}));
    REQUIRE(signal_count == 2);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(model.get<int>(1, 0) == 2);
    REQUIRE(model.get<int>(1, 1) == 7);
    REQUIRE(model.get<int>(1, 2) == 1);
  }

  TEST_CASE("remove") {
    auto model = ArrayTableModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        REQUIRE(false);
      }));
    REQUIRE_THROWS(model.remove(0));
    auto signal_count = 0;
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
        REQUIRE(add_operation->m_index == model.get_row_size() - 1);
      });
    REQUIRE_NOTHROW(model.push({1, 2}));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push({3, 4}));
    REQUIRE(signal_count == 2);
    REQUIRE_NOTHROW(model.push({5, 6}));
    REQUIRE(signal_count == 3);
    REQUIRE(model.get_row_size() == 3);
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto remove_operation = operation.get<TableModel::RemoveOperation>();
        REQUIRE((remove_operation != none));
        REQUIRE(remove_operation->m_index == 0);
      });
    REQUIRE_NOTHROW(model.remove(0));
    REQUIRE(signal_count == 4);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(model.get<int>(0, 0) == 3);
    REQUIRE(model.get<int>(0, 1) == 4);
    REQUIRE(model.get<int>(1, 0) == 5);
    REQUIRE(model.get<int>(1, 1) == 6);
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        REQUIRE(false);
      });
    REQUIRE_THROWS(model.remove(2));
  }

  TEST_CASE("move") {
    auto model = ArrayTableModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        REQUIRE(false);
      }));
    REQUIRE_THROWS(model.move(1, 3));
    auto signal_count = 0;
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
        REQUIRE(add_operation->m_index == model.get_row_size() - 1);
      });
    REQUIRE_NOTHROW(model.push({1, 2, 3}));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push({4, 5, 6}));
    REQUIRE(signal_count == 2);
    REQUIRE_NOTHROW(model.push({7, 8, 9}));
    REQUIRE(signal_count == 3);
    REQUIRE_NOTHROW(model.push({10, 11, 12}));
    REQUIRE(signal_count == 4);
    REQUIRE(model.get_row_size() == 4);
    auto source = 0;
    auto destination = 0;
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto move_operation = operation.get<TableModel::MoveOperation>();
        REQUIRE((move_operation != none));
        REQUIRE(move_operation->m_source == source);
        REQUIRE(move_operation->m_destination == destination);
      });
    REQUIRE_THROWS(model.move(0, 4));
    REQUIRE(signal_count == 4);
    REQUIRE_THROWS(model.move(5, 1));
    REQUIRE(signal_count == 4);
    source = 2;
    destination = 2;
    REQUIRE_NOTHROW(model.move(source, destination));
    REQUIRE(model.get<int>(0, 0) == 1);
    REQUIRE(model.get<int>(1, 0) == 4);
    REQUIRE(model.get<int>(2, 0) == 7);
    REQUIRE(model.get<int>(3, 0) == 10);
    source = 0;
    destination = 3;
    REQUIRE_NOTHROW(model.move(source, destination));
    REQUIRE(signal_count == 5);
    REQUIRE(model.get<int>(0, 0) == 4);
    REQUIRE(model.get<int>(0, 1) == 5);
    REQUIRE(model.get<int>(1, 0) == 7);
    REQUIRE(model.get<int>(1, 1) == 8);
    REQUIRE(model.get<int>(2, 0) == 10);
    REQUIRE(model.get<int>(2, 1) == 11);
    REQUIRE(model.get<int>(3, 0) == 1);
    REQUIRE(model.get<int>(3, 1) == 2);
    source = 3;
    destination = 1;
    REQUIRE_NOTHROW(model.move(source, destination));
    REQUIRE(signal_count == 6);
    REQUIRE(model.get<int>(0, 0) == 4);
    REQUIRE(model.get<int>(0, 1) == 5);
    REQUIRE(model.get<int>(1, 0) == 1);
    REQUIRE(model.get<int>(1, 1) == 2);
    REQUIRE(model.get<int>(2, 0) == 7);
    REQUIRE(model.get<int>(2, 1) == 8);
    REQUIRE(model.get<int>(3, 0) == 10);
    REQUIRE(model.get<int>(3, 1) == 11);
  }

  TEST_CASE("insert") {
    auto model = ArrayTableModel();
    auto signal_count = 0;
    auto index = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
        REQUIRE(add_operation->m_index == index);
      }));
    index = 0;
    REQUIRE_NOTHROW(model.insert({}, index));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(signal_count == 1);
    REQUIRE_THROWS(model.insert({1, 2, 3}, index));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(signal_count == 1);
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto remove_operation = operation.get<TableModel::RemoveOperation>();
        REQUIRE((remove_operation != none));
      });
    REQUIRE_NOTHROW(model.remove(index));
    REQUIRE(signal_count == 2);
    signal_count = 0;
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
        REQUIRE(add_operation->m_index == index);
      });
    index = 0;
    REQUIRE_NOTHROW(model.insert({1, 2, 3}, index));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.insert({4, 5, 6}, index));
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(signal_count == 2);
    REQUIRE(model.get<int>(0, 0) == 4);
    REQUIRE(model.get<int>(0, 1) == 5);
    REQUIRE(model.get<int>(1, 0) == 1);
    REQUIRE(model.get<int>(1, 1) == 2);
    index = -1;
    REQUIRE_THROWS(model.insert({7, 8, 9}, index));
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(signal_count == 2);
    index = 2;
    REQUIRE_NOTHROW(model.insert({7, 8, 9}, index));
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(signal_count == 3);
    REQUIRE(model.get<int>(0, 0) == 4);
    REQUIRE(model.get<int>(1, 0) == 1);
    REQUIRE(model.get<int>(2, 0) == 7);
    REQUIRE(model.get<int>(2, 1) == 8);
    index = 1;
    REQUIRE_THROWS(model.insert({10}, index));
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(signal_count == 3);
    REQUIRE_NOTHROW(model.insert({10, 11, 12}, index));
    REQUIRE(model.get_row_size() == 4);
    REQUIRE(signal_count == 4);
    REQUIRE(model.get<int>(0, 0) == 4);
    REQUIRE(model.get<int>(1, 0) == 10);
    REQUIRE(model.get<int>(2, 0) == 1);
    REQUIRE(model.get<int>(3, 0) == 7);
  }

  TEST_CASE("update") {
    auto model = ArrayTableModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        REQUIRE(false);
      }));
    REQUIRE(model.set(1, 3, 0) == QValidator::State::Invalid);
    auto signal_count = 0;
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
        REQUIRE(add_operation->m_index == model.get_row_size() - 1);
      });
    REQUIRE_NOTHROW(model.push({1, 2, 3}));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push({4, 5, 6}));
    REQUIRE(signal_count == 2);
    REQUIRE_NOTHROW(model.push({7, 8, 9}));
    REQUIRE(signal_count == 3);
    auto row = 0;
    auto column = 0;
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto update_operation = operation.get<TableModel::UpdateOperation>();
        REQUIRE((update_operation != none));
        REQUIRE(update_operation->m_row == row);
        REQUIRE(update_operation->m_column == column);
      });
    row = 0;
    column = 0;
    REQUIRE(model.set(row, column, 0) == QValidator::State::Acceptable);
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(signal_count == 4);
    REQUIRE(model.get<int>(row, column) == 0);
    row = 1;
    column = 1;
    REQUIRE(model.set(row, column, 10) == QValidator::State::Acceptable);
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(signal_count == 5);
    REQUIRE(model.get<int>(row, column) == 10);
    row = 2;
    column = 2;
    REQUIRE(model.set(row, column, 0) == QValidator::State::Acceptable);
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(signal_count == 6);
    REQUIRE(model.get<int>(row, column) == 0);
    row = 3;
    column = 3;
    REQUIRE(model.set(row, column, 0) == QValidator::State::Invalid);
    REQUIRE(signal_count == 6);
  }

  TEST_CASE("subtransactions") {
    auto model = ArrayTableModel();
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto transaction = operation.get<TableModel::Transaction>();
        REQUIRE((transaction != none));
        REQUIRE(transaction->size() == 5);
        REQUIRE(((*transaction)[0].get<TableModel::AddOperation>() != none));
        REQUIRE(((*transaction)[1].get<TableModel::UpdateOperation>() != none));
        REQUIRE(((*transaction)[2].get<TableModel::AddOperation>() != none));
        REQUIRE(((*transaction)[3].get<TableModel::RemoveOperation>() != none));
        REQUIRE(((*transaction)[4].get<TableModel::AddOperation>() != none));
      }));
    model.transact([&] {
      model.push({1, 2, 3});
      model.transact([&] {
        model.set(0, 0, 10);
        model.transact([&] {
          model.push({9, 0, 2});
          model.remove(1);
        });
        model.push({8, 9, 9});
      });
    });
    REQUIRE(signal_count == 1);
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        connection.disconnect();
        model.transact([&] {
          model.push({7, 8, 9});
        });
        auto transaction = operation.get<TableModel::Transaction>();
        REQUIRE((transaction != none));
        REQUIRE(transaction->size() == 2);
        REQUIRE(((*transaction)[0].get<TableModel::AddOperation>() != none));
        REQUIRE(((*transaction)[1].get<TableModel::AddOperation>() != none));
      });
    model.transact([&] {
      model.push({1, 2, 3});
      model.push({4, 5, 6});
    });
    REQUIRE(signal_count == 2);
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = operation.get<TableModel::AddOperation>();
        REQUIRE((add_operation != none));
        REQUIRE(add_operation->m_index == model.get_row_size() - 1);
      });
    model.transact([&] {
      model.push({1, 2, 3});
    });
    REQUIRE(signal_count == 3);
  }
}
