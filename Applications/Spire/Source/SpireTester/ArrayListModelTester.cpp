#include <doctest/doctest.h>
#include "Spire/Ui/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ArrayListModel") {
  TEST_CASE("push") {
    auto model = ArrayListModel();
    REQUIRE(model.get_size() == 0);
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<ListModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == model.get_size() - 1);
      }));
    REQUIRE_NOTHROW(model.push(3));
    REQUIRE(signal_count == 1);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get<int>(0) == 3);
    REQUIRE_NOTHROW(model.push(2));
    REQUIRE(signal_count == 2);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get<int>(1) == 2);
  }

  TEST_CASE("remove") {
    auto model = ArrayListModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        REQUIRE(false);
      }));
    REQUIRE_THROWS(model.remove(0));
    auto signal_count = 0;
    auto remove_count = 0;
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        ++signal_count;
        visit<ListModel>(operation,
          [&] (const ListModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == model.get_size() - 1);
          },
          [&] (const ListModel::RemoveOperation& remove_operation) {
            ++remove_count;
            REQUIRE(remove_operation.m_index == 0);
          });
      });
    REQUIRE_NOTHROW(model.push(1));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push(3));
    REQUIRE(signal_count == 2);
    REQUIRE_NOTHROW(model.push(5));
    REQUIRE(signal_count == 3);
    REQUIRE(model.get_size() == 3);
    REQUIRE_NOTHROW(model.remove(0));
    REQUIRE(signal_count == 4);
    REQUIRE(remove_count == 1);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get<int>(0) == 3);
    REQUIRE(model.get<int>(1) == 5);
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        REQUIRE(false);
      });
    REQUIRE_THROWS(model.remove(2));
  }

  TEST_CASE("move") {
    auto model = ArrayListModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        REQUIRE(false);
      }));
    REQUIRE_THROWS(model.move(1, 3));
    auto signal_count = 0;
    auto move_count = 0;
    auto source = 0;
    auto destination = 0;
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        ++signal_count;
        visit<ListModel>(operation,
          [&] (const ListModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == model.get_size() - 1);
          },
          [&] (const ListModel::MoveOperation& move_operation) {
            ++move_count;
            REQUIRE(move_operation.m_source == source);
            REQUIRE(move_operation.m_destination == destination);
          });
      });
    REQUIRE_NOTHROW(model.push(1));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push(4));
    REQUIRE(signal_count == 2);
    REQUIRE_NOTHROW(model.push(7));
    REQUIRE(signal_count == 3);
    REQUIRE_NOTHROW(model.push(10));
    REQUIRE(signal_count == 4);
    REQUIRE(model.get_size() == 4);
    REQUIRE_THROWS(model.move(0, 4));
    REQUIRE(signal_count == 4);
    REQUIRE_THROWS(model.move(5, 1));
    REQUIRE(signal_count == 4);
    source = 2;
    destination = 2;
    REQUIRE_NOTHROW(model.move(source, destination));
    REQUIRE(model.get<int>(0) == 1);
    REQUIRE(model.get<int>(1) == 4);
    REQUIRE(model.get<int>(2) == 7);
    REQUIRE(model.get<int>(3) == 10);
    source = 0;
    destination = 3;
    REQUIRE_NOTHROW(model.move(source, destination));
    REQUIRE(signal_count == 5);
    REQUIRE(model.get<int>(0) == 4);
    REQUIRE(model.get<int>(1) == 7);
    REQUIRE(model.get<int>(2) == 10);
    REQUIRE(model.get<int>(3) == 1);
    source = 3;
    destination = 1;
    REQUIRE_NOTHROW(model.move(source, destination));
    REQUIRE(signal_count == 6);
    REQUIRE(model.get<int>(0) == 4);
    REQUIRE(model.get<int>(1) == 1);
    REQUIRE(model.get<int>(2) == 7);
    REQUIRE(model.get<int>(3) == 10);
  }

  TEST_CASE("insert") {
    auto model = ArrayListModel();
    auto signal_count = 0;
    auto index = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<ListModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == index);
      }));
    index = 1;
    REQUIRE_THROWS(model.insert(1, index));
    REQUIRE(model.get_size() == 0);
    REQUIRE(signal_count == 0);
    index = 0;
    REQUIRE_NOTHROW(model.insert(1, index));
    REQUIRE(model.get_size() == 1);
    REQUIRE(signal_count == 1);
    REQUIRE(model.get<int>(0) == 1);
    index = 0;
    REQUIRE_NOTHROW(model.insert(2, index));
    REQUIRE(model.get_size() == 2);
    REQUIRE(signal_count == 2);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 1);
    index = -1;
    REQUIRE_THROWS(model.insert(3, index));
    REQUIRE(model.get_size() == 2);
    REQUIRE(signal_count == 2);
    index = 1;
    REQUIRE_NOTHROW(model.insert(3, index));
    REQUIRE(model.get_size() == 3);
    REQUIRE(signal_count == 3);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 3);
    REQUIRE(model.get<int>(2) == 1);
  }

  TEST_CASE("update") {
    auto model = ArrayListModel();
    auto signal_count = 0;
    auto update_count = 0;
    auto updated_index = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
        visit<ListModel>(operation,
          [&] (const ListModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == model.get_size() - 1);
          },
          [&] (const ListModel::UpdateOperation& update_operation) {
            ++update_count;
            REQUIRE(update_operation.m_index == updated_index);
          },
          [] (const auto&) {
            REQUIRE(false);
          });
      }));
    updated_index = 2;
    REQUIRE(model.set(updated_index, 0) == QValidator::State::Invalid);
    REQUIRE(model.get_size() == 0);
    REQUIRE(signal_count == 0);
    REQUIRE_NOTHROW(model.push(1));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push(4));
    REQUIRE(signal_count == 2);
    REQUIRE_NOTHROW(model.push(7));
    REQUIRE(signal_count == 3);
    updated_index = 0;
    REQUIRE(model.set(updated_index, 0) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 3);
    REQUIRE(signal_count == 4);
    REQUIRE(model.get<int>(updated_index) == 0);
    updated_index = 2;
    REQUIRE(model.set(updated_index, 10) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 3);
    REQUIRE(signal_count == 5);
    REQUIRE(model.get<int>(updated_index) == 10);
  }
  
  TEST_CASE("transactions") {
    auto model = ArrayListModel();
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        ++signal_count;
        auto transaction = get<ListModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 5);
        auto& operations = transaction->m_operations;
        REQUIRE(get<ListModel::AddOperation>(&operations[0]));
        REQUIRE(get<ListModel::UpdateOperation>(&operations[1]));
        REQUIRE(get<ListModel::AddOperation>(&operations[2]));
        REQUIRE(get<ListModel::RemoveOperation>(&operations[3]));
        REQUIRE(get<ListModel::AddOperation>(&operations[4]));
      }));
    model.transact([&] {
      model.push(1);
      model.transact([&] {
        model.set(0, 10);
        model.transact([&] {
          model.push(9);
          model.remove(1);
        });
        model.push(8);
      });
    });
    REQUIRE(signal_count == 1);
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        ++signal_count;
        connection.disconnect();
        model.transact([&] {
          model.push(7);
        });
        auto transaction = get<ListModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 2);
        auto& operations = transaction->m_operations;
        REQUIRE(get<ListModel::AddOperation>(&operations[0]));
        REQUIRE(get<ListModel::AddOperation>(&operations[1]));
      });
    model.transact([&] {
      model.push(1);
      model.push(4);
    });
    REQUIRE(signal_count == 2);
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<ListModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == model.get_size() - 1);
      });
    model.transact([&] {
      model.push(1);
    });
    REQUIRE(signal_count == 3);
  }
}
