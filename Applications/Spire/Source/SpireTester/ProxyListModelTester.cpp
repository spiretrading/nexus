#include <deque>
#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ProxyListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  template<typename T>
  void require_set_source_exception() {
    auto first =
      std::make_shared<ArrayListModel<int>>(std::vector<int>{1, 2, 3});
    auto second = std::make_shared<ArrayListModel<int>>(std::vector<int>{7, 8});
    auto proxy = ProxyListModel(first);
    auto is_throwing = true;
    auto operations = std::deque<ProxyListModel<int>::Operation>();
    auto connection = scoped_connection(proxy.connect_operation_signal(
      [&] (const auto& operation) {
        if(is_throwing && get<T>(&operation)) {
          throw std::runtime_error("Failed.");
        }
        operations.push_back(operation);
      }));
    REQUIRE_THROWS_AS(proxy.set_source(second), std::runtime_error);
    is_throwing = false;
    REQUIRE(proxy.get_source() == second);
    REQUIRE(proxy.get_size() == 2);
    REQUIRE(proxy.get(0) == 7);
    REQUIRE(proxy.get(1) == 8);
    operations.clear();
    first->push(4);
    REQUIRE(operations.empty());
    REQUIRE(proxy.get_size() == 2);
    second->push(9);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(2)
      });
    REQUIRE(proxy.get(2) == 9);
  }
}

TEST_SUITE("ProxyListModel") {
  TEST_CASE("constructor") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(1);
    source->push(2);
    source->push(3);
    auto proxy = ProxyListModel(source);
    REQUIRE(proxy.get_source() == source);
    REQUIRE(proxy.get_size() == 3);
    REQUIRE(proxy.get(0) == 1);
    REQUIRE(proxy.get(1) == 2);
    REQUIRE(proxy.get(2) == 3);
    REQUIRE_THROWS_AS(proxy.get(3), std::out_of_range);
    SUBCASE("insert") {
      proxy.insert(4, 0);
      REQUIRE(source->get(0) == 4);
      REQUIRE(proxy.get(0) == 4);
    }
    SUBCASE("remove") {
      proxy.remove(0);
      REQUIRE(source->get_size() == 2);
      REQUIRE(proxy.get(0) == 2);
    }
    SUBCASE("set") {
      proxy.set(1, 9);
      REQUIRE(source->get(1) == 9);
      REQUIRE(proxy.get(1) == 9);
    }
    SUBCASE("move") {
      proxy.move(0, 2);
      REQUIRE(source->get(2) == 1);
      REQUIRE(proxy.get(2) == 1);
    }
  }

  TEST_CASE("source_operations") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(1);
    auto proxy = ProxyListModel(source);
    auto operations = std::deque<ProxyListModel<int>::Operation>();
    auto connection = scoped_connection(proxy.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->push(2);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(1)
      });
    operations.clear();
    source->remove(0);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::PreRemoveOperation(0),
        ListModel<int>::RemoveOperation(0)
      });
    operations.clear();
    source->transact([&] {
      source->push(3);
      source->push(4);
    });
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(1),
        ListModel<int>::AddOperation(2)
      });
  }

  TEST_CASE("set_source") {
    auto first =
      std::make_shared<ArrayListModel<int>>(std::vector<int>{1, 2, 3});
    auto second = std::make_shared<ArrayListModel<int>>(std::vector<int>{7, 8});
    auto proxy = ProxyListModel(first);
    auto operations = std::deque<ProxyListModel<int>::Operation>();
    auto sizes = std::vector<int>();
    auto connection = scoped_connection(proxy.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
        sizes.push_back(proxy.get_size());
        if(auto pre_remove =
            get<ListModel<int>::PreRemoveOperation>(&operation)) {
          REQUIRE(pre_remove->m_index == proxy.get_size() - 1);
          REQUIRE(
            proxy.get(pre_remove->m_index) == first->get(pre_remove->m_index));
        } else if(auto remove =
            get<ListModel<int>::RemoveOperation>(&operation)) {
          REQUIRE(remove->m_index == proxy.get_size());
          REQUIRE_THROWS_AS(proxy.get(remove->m_index), std::out_of_range);
        } else if(auto add = get<ListModel<int>::AddOperation>(&operation)) {
          REQUIRE(add->m_index == proxy.get_size() - 1);
          REQUIRE(proxy.get(add->m_index) == second->get(add->m_index));
        }
      }));
    proxy.set_source(second);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::PreRemoveOperation(2),
        ListModel<int>::RemoveOperation(2),
        ListModel<int>::PreRemoveOperation(1),
        ListModel<int>::RemoveOperation(1),
        ListModel<int>::PreRemoveOperation(0),
        ListModel<int>::RemoveOperation(0),
        ListModel<int>::AddOperation(0),
        ListModel<int>::AddOperation(1)
      });
    REQUIRE(sizes == std::vector<int>{3, 3, 2, 2, 1, 1, 0, 1, 2, 2});
    REQUIRE(proxy.get_source() == second);
    REQUIRE(proxy.get_size() == 2);
    REQUIRE(proxy.get(0) == 7);
    REQUIRE(proxy.get(1) == 8);
    operations.clear();
    first->push(4);
    REQUIRE(operations.empty());
    REQUIRE(proxy.get_size() == 2);
    second->push(9);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(2)
      });
    REQUIRE(proxy.get(2) == 9);
  }

  TEST_CASE("set_source_remove_exception") {
    require_set_source_exception<ListModel<int>::RemoveOperation>();
  }

  TEST_CASE("set_source_add_exception") {
    require_set_source_exception<ListModel<int>::AddOperation>();
  }

  TEST_CASE("set_empty_source") {
    auto first = std::make_shared<ArrayListModel<int>>();
    auto second = std::make_shared<ArrayListModel<int>>();
    auto proxy = ProxyListModel(first);
    auto operations = std::deque<ProxyListModel<int>::Operation>();
    auto connection = scoped_connection(proxy.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    proxy.set_source(second);
    REQUIRE(operations.empty());
    REQUIRE(proxy.get_source() == second);
    REQUIRE(proxy.get_size() == 0);
  }
}
