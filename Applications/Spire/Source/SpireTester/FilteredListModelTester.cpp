#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("FilteredListModel") {
  TEST_CASE("filter") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) > 3;
      });
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get_size() == 2);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 1);
  }

  TEST_CASE("push") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(9);
    source->push(4);
    source->push(2);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) > 4;
      });
    REQUIRE(filtered_list.get(0) == 4);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(source->get(0) == 9);
    REQUIRE(source->get(1) == 4);
    REQUIRE(source->get(2) == 2);
    auto signal_count = 0;
    auto connection = scoped_connection(filtered_list.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        auto add_operation = get<ListModel<int>::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == filtered_list.get_size() - 1);
      }));
    source->push({10});
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_list.get(0) == 4);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(filtered_list.get_size() == 2);
    REQUIRE(source->get(0) == 9);
    REQUIRE(source->get(1) == 4);
    REQUIRE(source->get(2) == 2);
    REQUIRE(source->get(3) == 10);
    source->push(1);
    REQUIRE(signal_count == 1);
    REQUIRE(filtered_list.get(0) == 4);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(filtered_list.get(2) == 1);
    REQUIRE(filtered_list.get_size() == 3);
    REQUIRE(source->get(0) == 9);
    REQUIRE(source->get(1) == 4);
    REQUIRE(source->get(2) == 2);
    REQUIRE(source->get(3) == 10);
    REQUIRE(source->get(4) == 1);
  }

  TEST_CASE("insert") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(1);
    source->push(4);
    source->push(2);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) > 3;
      });
    REQUIRE(filtered_list.get(0) == 1);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(source->get(0) == 1);
    REQUIRE(source->get(1) == 4);
    REQUIRE(source->get(2) == 2);
    auto signal_count = 0;
    auto added_index = 0;
    auto connection = scoped_connection(filtered_list.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        auto add_operation = get<ListModel<int>::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == added_index);
      }));
    source->insert(9, 1);
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_list.get(0) == 1);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(filtered_list.get_size() == 2);
    REQUIRE(source->get(0) == 1);
    REQUIRE(source->get(1) == 9);
    REQUIRE(source->get(2) == 4);
    REQUIRE(source->get(3) == 2);
    added_index = 1;
    source->insert(0, 2);
    REQUIRE(signal_count == 1);
    REQUIRE(filtered_list.get(0) == 1);
    REQUIRE(filtered_list.get(1) == 0);
    REQUIRE(filtered_list.get(2) == 2);
    REQUIRE(filtered_list.get_size() == 3);
    REQUIRE(source->get(0) == 1);
    REQUIRE(source->get(1) == 9);
    REQUIRE(source->get(2) == 0);
    REQUIRE(source->get(3) == 4);
    REQUIRE(source->get(4) == 2);
    added_index = 3;
    source->insert(0, 5);
    REQUIRE(signal_count == 2);
    REQUIRE(filtered_list.get(0) == 1);
    REQUIRE(filtered_list.get(1) == 0);
    REQUIRE(filtered_list.get(2) == 2);
    REQUIRE(filtered_list.get(3) == 0);
    REQUIRE(filtered_list.get_size() == 4);
    REQUIRE(source->get(0) == 1);
    REQUIRE(source->get(1) == 9);
    REQUIRE(source->get(2) == 0);
    REQUIRE(source->get(3) == 4);
    REQUIRE(source->get(4) == 2);
    REQUIRE(source->get(5) == 0);
  }

  TEST_CASE("remove") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) > 3;
      });
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 1);
    auto signal_count = 0;
    auto connection = scoped_connection(filtered_list.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        visit(operation,
          [&] (const ListModel<int>::PreRemoveOperation& operation) {
            ++signal_count;
            REQUIRE(operation.m_index == 0);
          },
          [&] (const ListModel<int>::RemoveOperation& operation) {
            ++signal_count;
            REQUIRE(operation.m_index == 0);
          });
      }));
    source->remove(2);
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get_size() == 2);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 1);
    source->remove(1);
    REQUIRE(signal_count == 2);
    REQUIRE(filtered_list.get(0) == 1);
    REQUIRE(filtered_list.get_size() == 1);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 1);
    source->remove(1);
    REQUIRE(signal_count == 4);
    REQUIRE(filtered_list.get_size() == 0);
    REQUIRE(source->get(0) == 4);
  }

  TEST_CASE("move") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(6);
    source->push(1);
    source->push(3);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) > 3;
      });
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get(2) == 3);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 6);
    REQUIRE(source->get(4) == 1);
    REQUIRE(source->get(5) == 3);
    auto signal_count = 0;
    auto source_row = 0;
    auto destination_row = 0;
    auto connection = scoped_connection(filtered_list.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        auto move_operation = get<ListModel<int>::MoveOperation>(&operation);
        REQUIRE(move_operation != nullptr);
        REQUIRE(move_operation->m_source == source_row);
        REQUIRE(move_operation->m_destination == destination_row);
      }));
    source->move(3, 0);
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get(2) == 3);
    REQUIRE(source->get(0) == 6);
    REQUIRE(source->get(1) == 4);
    REQUIRE(source->get(2) == 2);
    REQUIRE(source->get(3) == 9);
    REQUIRE(source->get(4) == 1);
    REQUIRE(source->get(5) == 3);
    source->move(0, 4);
    REQUIRE(signal_count == 0);
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get(2) == 3);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 1);
    REQUIRE(source->get(4) == 6);
    REQUIRE(source->get(5) == 3);
    source_row = 0;
    destination_row = 2;
    source->move(1, 5);
    REQUIRE(signal_count == 1);
    REQUIRE(filtered_list.get(0) == 1);
    REQUIRE(filtered_list.get(1) == 3);
    REQUIRE(filtered_list.get(2) == 2);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 9);
    REQUIRE(source->get(2) == 1);
    REQUIRE(source->get(3) == 6);
    REQUIRE(source->get(4) == 3);
    REQUIRE(source->get(5) == 2);
    source_row = 1;
    destination_row = 0;
    source->move(4, 0);
    REQUIRE(signal_count == 2);
    REQUIRE(filtered_list.get(0) == 3);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get(2) == 2);
    REQUIRE(source->get(0) == 3);
    REQUIRE(source->get(1) == 4);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 1);
    REQUIRE(source->get(4) == 6);
    REQUIRE(source->get(5) == 2);
  }

  TEST_CASE("update") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) > 3;
      });
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 1);
    auto signal_count = 0;
    auto add_count = 0;
    auto pre_remove_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto connection = scoped_connection(filtered_list.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const ListModel<int>::AddOperation& operation) {
            ++add_count;
            REQUIRE(operation.m_index == 0);
          },
          [&] (const ListModel<int>::PreRemoveOperation& operation) {
            ++pre_remove_count;
            REQUIRE(operation.m_index == 2);
          },
          [&] (const ListModel<int>::RemoveOperation& operation) {
            ++remove_count;
            REQUIRE(operation.m_index == 2);
          },
          [&] (const ListModel<int>::UpdateOperation& operation) {
            ++update_count;
            REQUIRE(operation.m_index == 1);
          },
          [] (const auto&) {
            REQUIRE(false);
          });
      }));
    source->set(0, 0);
    REQUIRE(signal_count == 1);
    REQUIRE(add_count == 1);
    REQUIRE(filtered_list.get(0) == 0);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(filtered_list.get(2) == 1);
    REQUIRE(filtered_list.get_size() == 3);
    REQUIRE(source->get(0) == 0);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 1);
    source->set(3, 10);
    REQUIRE(signal_count == 3);
    REQUIRE(pre_remove_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(filtered_list.get(0) == 0);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(filtered_list.get_size() == 2);
    REQUIRE(source->get(0) == 0);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 10);
    source->set(1, 1);
    REQUIRE(signal_count == 4);
    REQUIRE(update_count == 1);
    REQUIRE(filtered_list.get(0) == 0);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get_size() == 2);
    REQUIRE(source->get(0) == 0);
    REQUIRE(source->get(1) == 1);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 10);
    source->set(3, 6);
    REQUIRE(signal_count == 4);
    REQUIRE(filtered_list.get(0) == 0);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get_size() == 2);
    REQUIRE(source->get(0) == 0);
    REQUIRE(source->get(1) == 1);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 6);
  }

  TEST_CASE("transaction") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) > 3;
      });
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(source->get(0) == 4);
    REQUIRE(source->get(1) == 2);
    REQUIRE(source->get(2) == 9);
    REQUIRE(source->get(3) == 1);
    auto signal_count = 0;
    auto start_count = 0;
    auto end_count = 0;
    auto add_count = 0;
    auto move_count = 0;
    auto pre_remove_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto connection = scoped_connection(filtered_list.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const ListModel<int>::StartTransaction&) {
            ++start_count;
          },
          [&] (const ListModel<int>::EndTransaction&) {
            ++end_count;
          },
          [&] (const ListModel<int>::AddOperation& operation) {
            ++add_count;
          },
          [&] (const ListModel<int>::MoveOperation& operation) {
            ++move_count;
            REQUIRE(operation.m_source == 2);
            REQUIRE(operation.m_destination == 0);
          },
          [&] (const ListModel<int>::PreRemoveOperation& operation) {
            ++pre_remove_count;
            REQUIRE(operation.m_index == 3);
          },
          [&] (const ListModel<int>::RemoveOperation& operation) {
            ++remove_count;
            REQUIRE(operation.m_index == 3);
          },
          [&] (const ListModel<int>::UpdateOperation& operation) {
            ++update_count;
            REQUIRE(operation.m_index == 0);
          });
      }));
    source->transact([&] {
      source->push(3);
      source->transact([&] {
        source->set(0, 0);
        source->transact([&] {
          source->insert(1, 1);
          source->remove(4);
        });
        source->set(0, 1);
      });
      source->move(2, 0);
    });
    REQUIRE(signal_count == 9);
    REQUIRE(start_count == 1);
    REQUIRE(end_count == 1);
    REQUIRE(add_count == 3);
    REQUIRE(move_count == 1);
    REQUIRE(pre_remove_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 1);
    REQUIRE(filtered_list.get(0) == 2);
    REQUIRE(filtered_list.get(1) == 1);
    REQUIRE(filtered_list.get(2) == 1);
    REQUIRE(filtered_list.get(3) == 3);
    REQUIRE(filtered_list.get_size() == 4);
  }

  TEST_CASE("set_filter") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(0);
    source->push(1);
    source->push(2);
    source->push(3);
    source->push(4);
    source->push(5);
    source->push(6);
    source->push(7);
    source->push(8);
    source->push(9);
    source->push(10);
    source->push(12);
    source->push(14);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) % 2 == 0;
      });
    REQUIRE(filtered_list.get_size() == 5);
    REQUIRE(filtered_list.get(0) == 1);
    REQUIRE(filtered_list.get(1) == 3);
    REQUIRE(filtered_list.get(2) == 5);
    REQUIRE(filtered_list.get(3) == 7);
    REQUIRE(filtered_list.get(4) == 9);
    auto operations = std::deque<ListModel<int>::Operation>();
    filtered_list.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    filtered_list.set_filter([] (const ListModel<int>& list, int index) {
      return list.get(index) % 2 != 0;
    });
    REQUIRE(filtered_list.get_size() == 8);
    REQUIRE(filtered_list.get(0) == 0);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(filtered_list.get(2) == 4);
    REQUIRE(filtered_list.get(3) == 6);
    REQUIRE(filtered_list.get(4) == 8);
    REQUIRE(filtered_list.get(5) == 10);
    REQUIRE(filtered_list.get(6) == 12);
    REQUIRE(filtered_list.get(7) == 14);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(0),
        ListModel<int>::PreRemoveOperation(1),
        ListModel<int>::RemoveOperation(1),
        ListModel<int>::AddOperation(1),
        ListModel<int>::PreRemoveOperation(2),
        ListModel<int>::RemoveOperation(2),
        ListModel<int>::AddOperation(2),
        ListModel<int>::PreRemoveOperation(3),
        ListModel<int>::RemoveOperation(3),
        ListModel<int>::AddOperation(3),
        ListModel<int>::PreRemoveOperation(4),
        ListModel<int>::RemoveOperation(4),
        ListModel<int>::AddOperation(4),
        ListModel<int>::PreRemoveOperation(5),
        ListModel<int>::RemoveOperation(5),
        ListModel<int>::AddOperation(5),
        ListModel<int>::AddOperation(6),
        ListModel<int>::AddOperation(7)
      });
  }

  TEST_CASE("set_filter_exclusive_trailing") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(0);
    source->push(1);
    source->push(2);
    source->push(3);
    source->push(4);
    source->push(5);
    source->push(6);
    source->push(7);
    source->push(8);
    source->push(9);
    source->push(11);
    source->push(13);
    source->push(15);
    auto filtered_list = FilteredListModel(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) % 2 == 0;
      });
    REQUIRE(filtered_list.get_size() == 8);
    REQUIRE(filtered_list.get(0) == 1);
    REQUIRE(filtered_list.get(1) == 3);
    REQUIRE(filtered_list.get(2) == 5);
    REQUIRE(filtered_list.get(3) == 7);
    REQUIRE(filtered_list.get(4) == 9);
    REQUIRE(filtered_list.get(5) == 11);
    REQUIRE(filtered_list.get(6) == 13);
    REQUIRE(filtered_list.get(7) == 15);
    auto operations = std::deque<ListModel<int>::Operation>();
    filtered_list.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    filtered_list.set_filter([] (const ListModel<int>& list, int index) {
      return list.get(index) % 2 != 0;
    });
    REQUIRE(filtered_list.get_size() == 5);
    REQUIRE(filtered_list.get(0) == 0);
    REQUIRE(filtered_list.get(1) == 2);
    REQUIRE(filtered_list.get(2) == 4);
    REQUIRE(filtered_list.get(3) == 6);
    REQUIRE(filtered_list.get(4) == 8);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(0),
        ListModel<int>::PreRemoveOperation(1),
        ListModel<int>::RemoveOperation(1),
        ListModel<int>::AddOperation(1),
        ListModel<int>::PreRemoveOperation(2),
        ListModel<int>::RemoveOperation(2),
        ListModel<int>::AddOperation(2),
        ListModel<int>::PreRemoveOperation(3),
        ListModel<int>::RemoveOperation(3),
        ListModel<int>::AddOperation(3),
        ListModel<int>::PreRemoveOperation(4),
        ListModel<int>::RemoveOperation(4),
        ListModel<int>::AddOperation(4),
        ListModel<int>::PreRemoveOperation(5),
        ListModel<int>::RemoveOperation(5),
        ListModel<int>::PreRemoveOperation(5),
        ListModel<int>::RemoveOperation(5),
        ListModel<int>::PreRemoveOperation(5),
        ListModel<int>::RemoveOperation(5),
        ListModel<int>::PreRemoveOperation(5),
        ListModel<int>::RemoveOperation(5)
      });
  }

  TEST_CASE("reentrant") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(0);
    source->push(1);
    source->push(2);
    source->push(3);
    auto filtered_list =
      FilteredListModel(source, [] (const auto& list, auto index) {
        return false;
      });
    REQUIRE(filtered_list.get_size() == 4);
    auto is_filter_reset = false;
    filtered_list.connect_operation_signal([&] (const auto& operation) {
      visit(operation,
        [&] (const ListModel<int>::PreRemoveOperation& operation) {
          if(std::exchange(is_filter_reset, true)) {
            return;
          }
          filtered_list.set_filter([] (const auto& list, auto index) {
            return list.get(index) % 2 != 0;
          });
        });
    });
    filtered_list.set_filter([] (const auto& list, auto index) {
      return list.get(index) % 2 == 0;
    });
    REQUIRE(filtered_list.get_size() == 2);
    REQUIRE(filtered_list.get(0) == 0);
    REQUIRE(filtered_list.get(1) == 2);
  }

  TEST_CASE("source_remove_consistency") {
    auto source = std::make_shared<ArrayListModel<int>>();;
    source->push(0);
    source->push(1);
    source->push(2);
    auto base_filtered_list = std::make_shared<FilteredListModel<int>>(
      source, [] (const auto& list, auto index) {
        return list.get(index) % 2 == 0;
      });
    auto filtered_list = FilteredListModel(
      base_filtered_list, [] (const auto& list, auto index) {
        return false;
      });
    filtered_list.connect_operation_signal([&] (const auto& operation) {
      visit(operation,
        [&] (const ListModel<int>::PreRemoveOperation& operation) {
          REQUIRE(filtered_list.get_size() == 1);
        });
    });
    source->remove(1);
  }

  TEST_CASE("chained_filters") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto base_filtered_list = std::make_shared<FilteredListModel<int>>(source,
      [] (const ListModel<int>& list, int index) {
        return list.get(index) > 5;
      });
    auto filtered_list = FilteredListModel(base_filtered_list,
      [] (const ListModel<int>& list, int index) {
        return false;
      });
    filtered_list.connect_operation_signal(
      [&] (const FilteredListModel<int>::Operation& operation) {
        visit(operation,
          [&] (const FilteredListModel<int>::RemoveOperation& operation) {
            REQUIRE(filtered_list.get_size() == 2);
            REQUIRE(filtered_list.get(0) == 2);
            REQUIRE(filtered_list.get(1) == 1);
          });
      });
    base_filtered_list->remove(0);
  }
}
