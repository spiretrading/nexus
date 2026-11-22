#include <deque>
#include <random>
#include <set>
#include <string>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/UnionListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  struct Entry {
    int m_id;
    std::string m_text;
  };
}

TEST_SUITE("UnionListModel") {
  TEST_CASE("sources") {
    auto model = UnionListModel<int>();
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS_AS(model.get(0), std::out_of_range);
    REQUIRE_THROWS_AS(model.get(-1), std::out_of_range);
    auto first =
      std::make_shared<ArrayListModel<int>>(std::vector{1, 2, 1});
    auto second = std::make_shared<ArrayListModel<int>>(std::vector{2, 3});
    model.add_source(first);
    model.add_source(second);
    model.add_source(first);
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get(0) == 1);
    REQUIRE(model.get(1) == 2);
    REQUIRE(model.get(2) == 3);
    first->remove(0);
    REQUIRE(model.get_size() == 3);
    first->remove(1);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0) == 2);
    model.remove_source(first);
    REQUIRE(model.get_size() == 2);
    first->push(4);
    REQUIRE(model.get_size() == 2);
    model.remove_source(first);
    model.remove_source(second);
    REQUIRE(model.get_size() == 0);
    second->push(5);
    REQUIRE(model.get_size() == 0);
    model.add_source(first);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0) == 2);
    REQUIRE(model.get(1) == 4);
    REQUIRE(model.push(9) == QValidator::Invalid);
    REQUIRE(model.set(0, 9) == QValidator::Invalid);
    REQUIRE(model.remove(0) == QValidator::Invalid);
  }

  TEST_CASE("source_operations") {
    auto source = std::make_shared<ArrayListModel<int>>(std::vector{1, 2});
    auto model = UnionListModel<int>();
    model.add_source(source);
    auto operations = std::deque<ListModel<int>::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->insert(3, 0);
    REQUIRE(model.get(2) == 3);
    require_list_transaction<int>(
      operations, {ListModel<int>::AddOperation(2)});
    operations.clear();
    source->push(3);
    REQUIRE(operations.empty());
    source->move(0, 2);
    source->move(3, 0);
    REQUIRE(operations.empty());
    source->remove(0);
    REQUIRE(operations.empty());
    source->remove(2);
    require_list_transaction<int>(operations,
      {ListModel<int>::PreRemoveOperation(2),
       ListModel<int>::RemoveOperation(2)});
    operations.clear();
    source->transact([&] {
      source->set(0, 2);
      source->push(4);
      source->push(4);
    });
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0) == 2);
    REQUIRE(model.get(1) == 4);
    require_list_transaction<int>(operations,
      {ListModel<int>::PreRemoveOperation(0),
       ListModel<int>::RemoveOperation(0),
       ListModel<int>::AddOperation(1)});
    operations.clear();
    source->set(1, 5);
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get(2) == 5);
    require_list_transaction<int>(operations,
      {ListModel<int>::UpdateOperation(0, 2, 2),
       ListModel<int>::AddOperation(2)});
  }

  TEST_CASE("custom_equality") {
    auto hash = [] (const auto&) {
      return std::size_t(0);
    };
    auto is_equal = [] (const auto& left, const auto& right) {
      return left.m_id == right.m_id;
    };
    auto model = UnionListModel<Entry, decltype(hash), decltype(is_equal)>();
    auto first = std::make_shared<ArrayListModel<Entry>>();
    auto second = std::make_shared<ArrayListModel<Entry>>();
    first->push(Entry(1, "first"));
    second->push(Entry(1, "second"));
    second->push(Entry(2, "distinct"));
    model.add_source(first);
    model.add_source(second);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0).m_text == "first");
    auto updates = std::vector<std::pair<std::string, std::string>>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        visit(operation, [&] (const ListModel<Entry>::UpdateOperation& update) {
          updates.emplace_back(
            update.get_previous().m_text, update.get_value().m_text);
          REQUIRE(
            model.get(update.m_index).m_text == update.get_value().m_text);
        });
      }));
    second->set(0, Entry(1, "hidden"));
    REQUIRE(updates.empty());
    REQUIRE(model.get(0).m_text == "first");
    first->set(0, Entry(1, "updated"));
    REQUIRE(model.get(0).m_text == "updated");
    REQUIRE(updates.back() ==
      std::pair(std::string("first"), std::string("updated")));
    first->set(0, Entry(2, "overlap"));
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0).m_text == "hidden");
    REQUIRE(model.get(1).m_text == "distinct");
    REQUIRE(updates.back() ==
      std::pair(std::string("updated"), std::string("hidden")));
    model.remove_source(second);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_id == 2);
    REQUIRE(model.get(0).m_text == "overlap");
    model.remove_source(first);
    REQUIRE(model.get_size() == 0);
  }

  TEST_CASE("hashed_lookup") {
    auto comparisons = 0;
    auto is_equal = [&] (auto left, auto right) {
      ++comparisons;
      return left == right;
    };
    auto model = UnionListModel<
      int, std::hash<int>, decltype(is_equal)>(std::hash<int>(), is_equal);
    auto source = std::make_shared<ArrayListModel<int>>();
    model.add_source(source);
    for(auto i = 0; i != 1024; ++i) {
      source->push(i);
    }
    comparisons = 0;
    source->push(1023);
    source->set(1024, 1022);
    REQUIRE(model.get_size() == 1024);
    REQUIRE(comparisons < 64);
  }

  TEST_CASE("mixed_operations") {
    auto model = UnionListModel<int>();
    auto sources = std::vector<std::shared_ptr<ArrayListModel<int>>>();
    auto registered = std::vector<bool>(4, false);
    for(auto i = 0; i != 4; ++i) {
      sources.push_back(std::make_shared<ArrayListModel<int>>());
    }
    auto mirror = std::vector<int>();
    auto depth = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        visit(operation,
          [&] (ListModel<int>::StartTransaction) {
            ++depth;
          },
          [&] (ListModel<int>::EndTransaction) {
            --depth;
            REQUIRE(depth == 0);
          },
          [&] (const ListModel<int>::AddOperation& add) {
            mirror.insert(mirror.begin() + add.m_index, model.get(add.m_index));
          },
          [&] (const ListModel<int>::PreRemoveOperation& remove) {
            REQUIRE(mirror.at(remove.m_index) == model.get(remove.m_index));
            REQUIRE(std::ranges::equal(mirror, std::as_const(model)));
          },
          [&] (const ListModel<int>::RemoveOperation& remove) {
            mirror.erase(mirror.begin() + remove.m_index);
          },
          [&] (const ListModel<int>::UpdateOperation& update) {
            REQUIRE(mirror.at(update.m_index) == update.get_previous());
            REQUIRE(model.get(update.m_index) == update.get_value());
            mirror[update.m_index] = update.get_value();
          });
      }));
    auto random = std::mt19937(762);
    for(auto i = 0; i != 500; ++i) {
      auto index = random() % sources.size();
      auto& source = sources[index];
      auto value = static_cast<int>(random() % 12);
      auto operation = random() % 6;
      if(operation == 0) {
        model.add_source(source);
        registered[index] = true;
      } else if(operation == 1) {
        model.remove_source(source);
        registered[index] = false;
      } else if(operation == 2) {
        source->insert(
          value, static_cast<int>(random() % (source->get_size() + 1)));
      } else if(operation == 3) {
        if(source->get_size() != 0) {
          source->remove(static_cast<int>(random() % source->get_size()));
        }
      } else if(operation == 4) {
        if(source->get_size() != 0) {
          source->set(static_cast<int>(random() % source->get_size()), value);
        }
      } else if(operation == 5) {
        if(source->get_size() != 0) {
          source->move(static_cast<int>(random() % source->get_size()),
            static_cast<int>(random() % source->get_size()));
        }
      }
      auto expected = std::set<int>();
      for(auto j = 0; j != static_cast<int>(sources.size()); ++j) {
        if(registered[j]) {
          for(auto value : std::as_const(*sources[j])) {
            expected.insert(value);
          }
        }
      }
      auto actual = std::set<int>();
      for(auto value : std::as_const(model)) {
        actual.insert(value);
      }
      REQUIRE(actual == expected);
      REQUIRE(model.get_size() == static_cast<int>(expected.size()));
      REQUIRE(std::ranges::equal(mirror, std::as_const(model)));
      REQUIRE(depth == 0);
    }
  }
}
