#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/SpireTester/TableModelTester.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  struct Person {
    std::string m_name;
    int m_age;
  };

  AnyRef extract(Person& person, int column) {
    if(column == 0) {
      return AnyRef(person.m_name);
    }
    return AnyRef(person.m_age);
  }
}

TEST_SUITE("ListToTableModel") {
  TEST_CASE("accessor") {
    auto source = std::make_shared<ArrayListModel<Person>>();
    source->push({"Skylar", 44});
    source->push({"Kevin", 33});
    source->push({"Joel", 22});
    auto table = ListToTableModel(source, 2, &extract);
    REQUIRE(table.get_row_size() == 3);
    REQUIRE(table.get_column_size() == 2);
    SUBCASE("get") {
      REQUIRE(table.get<std::string>(0, 0) == "Skylar");
      REQUIRE(table.get<int>(0, 1) == 44);
      REQUIRE(table.get<std::string>(1, 0) == "Kevin");
      REQUIRE(table.get<int>(1, 1) == 33);
      REQUIRE(table.get<std::string>(2, 0) == "Joel");
      REQUIRE(table.get<int>(2, 1) == 22);
    }
    SUBCASE("set") {
      table.set(0, 0, std::string("Joe"));
      table.set(0, 1, 55);
      REQUIRE(source->get(0).m_name == "Joe");
      REQUIRE(source->get(0).m_age == 55);
    }
  }

  TEST_CASE("operations") {
    auto source = std::make_shared<ArrayListModel<Person>>();
    source->push({"Skylar", 44});
    auto table =
      ListToTableModel(source, 2, &extract);
    auto operations = std::deque<TableModel::Operation>();
    table.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    source->set(0, {"Joe", 55});
    require_transaction(operations,
      {
        TableModel::UpdateOperation(
          0, 0, std::string("Skylar"), std::string("Joe")),
        TableModel::UpdateOperation(0, 1, 44, 55)
      });
  }
}
