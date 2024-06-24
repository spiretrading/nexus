#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TranslatedListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  template<typename... F>
  decltype(auto) test_operation(
      const ListModel<int>::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }
}

TEST_SUITE("TranslatedListModel") {
  TEST_CASE("translate") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto translation = TranslatedListModel(source);
    auto operations = std::deque<ListModel<int>::Operation>();
    translation.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation,
      [&] (const ListModel<int>::MoveOperation& operation) {
        REQUIRE(operation.m_source == 3);
        REQUIRE(operation.m_destination == 0);
      });
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 4);
    REQUIRE(translation.get(2) == 2);
    REQUIRE(translation.get(3) == 9);
    REQUIRE_THROWS(translation.move(6, 2));
    REQUIRE(operations.empty());
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    test_operation(operation,
      [&] (const ListModel<int>::MoveOperation& operation) {
        REQUIRE(operation.m_source == 1);
        REQUIRE(operation.m_destination == 2);
      });
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
  }

  TEST_CASE("transaction") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(10);
    source->push(9);
    source->push(1);
    source->push(6);
    auto translation = TranslatedListModel(source);
    auto operations = std::deque<ListModel<int>::Operation>();
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
    require_list_transaction<int>(operations,
      {
        ListModel<int>::MoveOperation(4, 0),
        ListModel<int>::MoveOperation(1, 2),
        ListModel<int>::MoveOperation(3, 5),
        ListModel<int>::MoveOperation(4, 3)
      });
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 6);
    REQUIRE(translation.get(4) == 9);
    REQUIRE(translation.get(5) == 10);
  }

  TEST_CASE("transaction_with_one_operation") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(1);
    source->push(6);
    auto signal_count = 0;
    auto translation = TranslatedListModel(source);
    auto operations = std::deque<ListModel<int>::Operation>();
    translation.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    translation.transact([&] {
      translation.move(0, 2);
      translation.transact([] {});
    });
    require_list_transaction<int>(operations,
      {
        ListModel<int>::MoveOperation(0, 2)
      });
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 6);
    REQUIRE(translation.get(2) == 4);
  }

  TEST_CASE("push_from_source") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto translation = TranslatedListModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
    auto signal_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        auto add_operation = get<ListModel<int>::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == translation.get_size() - 1);
      }));
    source->push({6});
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
    REQUIRE(translation.get(4) == 6);
    REQUIRE(translation.get_size() == 5);
  }

  TEST_CASE("insert_from_source") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto translation = TranslatedListModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
    auto signal_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        auto add_operation = get<ListModel<int>::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == 3);
      }));
    source->insert(6, 2);
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 6);
    REQUIRE(translation.get(4) == 9);
    REQUIRE(translation.get_size() == 5);
  }

  TEST_CASE("move_from_source") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto translation = TranslatedListModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
    auto signal_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
      }));
    source->move(1, 3);
    REQUIRE(signal_count == 0);
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
    source->move(2, 0);
    REQUIRE(signal_count == 0);
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
  }

  TEST_CASE("trivial_move") {
    auto source = std::make_shared<ArrayListModel<std::string>>();
    source->push(std::string("A"));
    source->push(std::string("B"));
    source->push(std::string("C"));
    auto translation = TranslatedListModel<std::string>(source);
    auto operations = std::deque<ListModel<std::string>::Operation>();
    translation.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    source->move(0, 2);
    REQUIRE(operations.size() == 0);
    source->set(2, std::string("D"));
    require_list_transaction<std::string>(operations,
      {
        ListModel<std::string>::UpdateOperation(0, "", ""),
      });
    REQUIRE(translation.get(0) == "D");
    REQUIRE(translation.get(1) == "B");
    REQUIRE(translation.get(2) == "C");
  }

  TEST_CASE("remove_from_source") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto translation = TranslatedListModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(1, 2));
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
    auto signal_count = 0;
    auto removed_index = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        visit(operation,
          [&] (const ListModel<int>::PreRemoveOperation& operation) {
            ++signal_count;
            REQUIRE(operation.m_index == removed_index);
          },
          [&] (const ListModel<int>::RemoveOperation& operation) {
            ++signal_count;
            REQUIRE(operation.m_index == removed_index);
          });
      }));
    removed_index = 2;
    source->remove(0);
    REQUIRE(signal_count == 2);
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 9);
    REQUIRE(translation.get_size() == 3);
    REQUIRE_THROWS(translation.get(3));
    removed_index = 0;
    source->remove(2);
    REQUIRE(signal_count == 4);
    REQUIRE(translation.get(0) == 2);
    REQUIRE(translation.get(1) == 9);
    REQUIRE(translation.get_size() == 2);
    REQUIRE_THROWS(translation.get(2));
  }

  TEST_CASE("update_from_source") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto translation = TranslatedListModel(source);
    REQUIRE_NOTHROW(translation.move(3, 0));
    REQUIRE_NOTHROW(translation.move(2, 1));
    auto signal_count = 0;
    auto updated_index = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        auto update_operation = get<ListModel<int>::UpdateOperation>(&operation);
        REQUIRE(update_operation != nullptr);
        REQUIRE(update_operation->m_index == updated_index);
      }));
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
    updated_index = 0;
    source->set(3, 10);
    REQUIRE(signal_count == 1);
    REQUIRE(translation.get(0) == 10);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 9);
    updated_index = 2;
    source->set(0, 0);
    REQUIRE(signal_count == 2);
    REQUIRE(translation.get(0) == 10);
    REQUIRE(translation.get(1) == 2);
    REQUIRE(translation.get(2) == 0);
    REQUIRE(translation.get(3) == 9);
  }

  TEST_CASE("set_translation") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(2);
    source->push(1);
    auto translation = TranslatedListModel(source);
    translation.move(1, 0);
    translation.set(0, 10);
    REQUIRE(source->get(0) == 2);
    REQUIRE(source->get(1) == 10);
  }

  TEST_CASE("transaction_from_source") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(2);
    source->push(1);
    auto translation = TranslatedListModel(source);
    REQUIRE_NOTHROW(translation.move(1, 0));
    REQUIRE(translation.get(0) == 1);
    REQUIRE(translation.get(1) == 2);
    auto operations = std::deque<ListModel<int>::Operation>();
    translation.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    source->transact([&] {
      source->push({4});
      source->transact([&] {
        source->set(0, 10);
        source->transact([&] {
          source->insert(6, 1);
          source->remove(2);
        });
        source->push({0});
      });
      source->move(3, 0);
    });
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(2),
        ListModel<int>::UpdateOperation(1, 0, 0),
        ListModel<int>::AddOperation(0),
        ListModel<int>::PreRemoveOperation(1),
        ListModel<int>::RemoveOperation(1),
        ListModel<int>::AddOperation(3)
      });
    REQUIRE(translation.get(0) == 6);
    REQUIRE(translation.get(1) == 10);
    REQUIRE(translation.get(2) == 4);
    REQUIRE(translation.get(3) == 0);
    REQUIRE(translation.get_size() == 4);
  }

  TEST_CASE("translate_mixing_with_source_operation") {
    auto source = std::make_shared<ArrayListModel<int>>();
    auto translation = TranslatedListModel(source);
    REQUIRE(translation.get_size() == 0);
    auto signal_count = 0;
    auto add_count = 0;
    auto move_count = 0;
    auto pre_remove_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto connection = scoped_connection(translation.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const ListModel<int>::AddOperation& operation) {
            ++add_count;
          },
          [&] (const ListModel<int>::MoveOperation& operation) {
            ++move_count;
            REQUIRE(operation.m_source == 1);
            REQUIRE(operation.m_destination == 0);
          },
          [&] (const ListModel<int>::PreRemoveOperation& operation) {
            ++pre_remove_count;
            REQUIRE(operation.m_index == 1);
          },
          [&] (const ListModel<int>::RemoveOperation& operation) {
            ++remove_count;
            REQUIRE(operation.m_index == 1);
          },
          [&] (const ListModel<int>::UpdateOperation& operation) {
            ++update_count;
            REQUIRE(operation.m_index == 0);
          });
      }));
    source->push(4);
    REQUIRE(signal_count == 1);
    REQUIRE(add_count == 1);
    REQUIRE(translation.get_size() == 1);
    REQUIRE(translation.get(0) == 4);
    source->push(2);
    REQUIRE(signal_count == 2);
    REQUIRE(add_count == 2);
    REQUIRE(translation.get_size() == 2);
    REQUIRE(translation.get(0) == 4);
    REQUIRE(translation.get(1) == 2);
    REQUIRE_NOTHROW(translation.move(1, 0));
    REQUIRE(signal_count == 3);
    REQUIRE(move_count == 1);
    REQUIRE(translation.get(0) == 2);
    REQUIRE(translation.get(1) == 4);
    source->set(1, 0);
    REQUIRE(signal_count == 4);
    REQUIRE(update_count == 1);
    REQUIRE(translation.get(0) == 0);
    REQUIRE(translation.get(1) == 4);
    source->remove(0);
    REQUIRE(signal_count == 6);
    REQUIRE(pre_remove_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(translation.get(0) == 0);
    REQUIRE(translation.get_size() == 1);
  }
}
