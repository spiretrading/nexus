#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/RowViewListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  template<typename... F>
  decltype(auto) test_operation(
      const RowViewListModel<int>::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }
}

TEST_SUITE("RowViewListModel") {
  TEST_CASE("construct") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    auto model1 = RowViewListModel<int>(source, 2);
    REQUIRE(model1.get_size() == 0);
    auto model2 = RowViewListModel<int>(source, -1);
    REQUIRE(model2.get_size() == 0);
    auto model3 = RowViewListModel<int>(source, 0);
    REQUIRE(model3.get_size() == 3);
    REQUIRE(model3.get(0) == 1);
    REQUIRE(model3.get(1) == 2);
    REQUIRE(model3.get(2) == 3);
  }

  TEST_CASE("update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    auto model = RowViewListModel<int>(source, 1);
    REQUIRE(model.get(0) == 4);
    REQUIRE(model.get(1) == 5);
    REQUIRE(model.get(2) == 6);
    auto operations = std::deque<RowViewListModel<int>::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    REQUIRE(model.set(4, 0) == QValidator::State::Invalid);
    REQUIRE(operations.empty());
    REQUIRE(model.set(-1, 0) == QValidator::State::Invalid);
    REQUIRE(operations.empty());
    REQUIRE(model.set(2, 0) == QValidator::State::Acceptable);
    REQUIRE(model.get(0) == 4);
    REQUIRE(model.get(1) == 5);
    REQUIRE(model.get(2) == 0);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation,
      [&] (const RowViewListModel<int>::UpdateOperation& operation) {
        REQUIRE(operation.m_index == 2);
        REQUIRE(operation.get_previous() == 6);
        REQUIRE(operation.get_value() == 0);
      });
  }

  TEST_CASE("source_add") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1});
    source->push({2});
    auto invalid_model = RowViewListModel<int>(source, 3);
    REQUIRE(invalid_model.get_size() == 0);
    auto operations1 = std::deque<RowViewListModel<int>::Operation>();
    auto connection1 = scoped_connection(invalid_model.connect_operation_signal(
      [&] (const auto& operation) {
        operations1.push_back(operation);
      }));
    auto model = RowViewListModel<int>(source, 1);
    REQUIRE(model.get(0) == 2);
    auto operations2 = std::deque<RowViewListModel<int>::Operation>();
    auto connection2 = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations2.push_back(operation);
      }));
    source->push({3});
    REQUIRE(operations1.empty());
    REQUIRE(operations2.empty());
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(model.get(0) == 2);
    source->insert({4}, 1);
    REQUIRE(operations1.empty());
    REQUIRE(operations2.empty());
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(model.get(0) == 2);
    source->insert({5}, 0);
    REQUIRE(operations1.empty());
    REQUIRE(operations2.empty());
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(model.get(0) == 2);
    source->push({6});
    REQUIRE(operations1.empty());
    REQUIRE(operations2.empty());
    REQUIRE(invalid_model.get_size() == 0);
    REQUIRE(model.get(0) == 2);
  }

  TEST_CASE("source_remove") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1});
    source->push({2});
    source->push({3});
    source->push({4});
    source->push({5});
    auto model = RowViewListModel<int>(source, 2);
    REQUIRE(model.get(0) == 3);
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
      }));
    source->remove(3);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get(0) == 3);
    source->remove(0);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get(0) == 3);
    source->remove(1);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS(model.get(0));
    source->insert({6}, 2);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS(model.get(0));
  }

  TEST_CASE("source_move") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({0});
    source->push({1});
    source->push({2});
    source->push({3});
    source->push({4});
    source->push({5});
    auto model = RowViewListModel<int>(source, 2);
    REQUIRE(model.get(0) == 2);
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        ++signal_count;
      }));
    source->move(3, 5);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get(0) == 2);
    source->move(2, 4);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get(0) == 2);
    source->move(0, 5);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get(0) == 2);
    source->move(4, 1);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get(0) == 2);
    source->move(3, 0);
    REQUIRE(signal_count == 0);
    REQUIRE(model.get(0) == 2);
  }

  TEST_CASE("source_update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    auto model = RowViewListModel<int>(source, 1);
    REQUIRE(model.get(0) == 4);
    REQUIRE(model.get(1) == 5);
    REQUIRE(model.get(2) == 6);
    auto operations = std::deque<RowViewListModel<int>::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->set(0, 0, 0);
    REQUIRE(operations.empty());
    source->set(1, 0, 0);
    REQUIRE(model.get(0) == 0);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation,
      [&] (const RowViewListModel<int>::UpdateOperation& operation) {
        REQUIRE(operation.m_index == 0);
        REQUIRE(operation.get_previous() == 4);
        REQUIRE(operation.get_value() == 0);
      });
    source->set(1, 2, 10);
    REQUIRE(model.get(2) == 10);
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    test_operation(operation,
      [&] (const RowViewListModel<int>::UpdateOperation& operation) {
        REQUIRE(operation.m_index == 2);
        REQUIRE(operation.get_previous() == 6);
        REQUIRE(operation.get_value() == 10);
      });
  }

  TEST_CASE("source_transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2, 3});
    source->push({4, 5, 6});
    auto model = RowViewListModel<int>(source, 1);
    auto operations = std::deque<RowViewListModel<int>::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
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
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    test_operation(operation,
      [&] (const RowViewListModel<int>::AddOperation& operation) {
        ++add_count;
      },
      [&] (const RowViewListModel<int>::MoveOperation& operation) {
        ++move_count;
      },
      [&] (const RowViewListModel<int>::RemoveOperation& operation) {
        ++remove_count;
      },
      [&] (const RowViewListModel<int>::UpdateOperation& operation) {
        ++update_count;
      });
    REQUIRE(add_count == 0);
    REQUIRE(move_count == 0);
    REQUIRE(remove_count == 0);
    REQUIRE(update_count == 3);
    REQUIRE(model.get(0) == 0);
    REQUIRE(model.get(1) == 0);
    REQUIRE(model.get(2) == 0);
  }
}
