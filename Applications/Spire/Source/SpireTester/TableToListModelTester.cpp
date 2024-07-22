#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/TableToListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("TableToListModel") {
  TEST_CASE("update") {
    auto table = std::make_shared<ArrayTableModel>();
    auto list = TableToListModel(table);
    auto operations = std::deque<ListModel<RowView>::Operation>();
    list.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    table->transact([&] {
      table->push({1, 2, 3});
      table->push({3, 2, 1});
      table->push({10, 20, 30});
    });
    require_list_transaction<RowView>(operations, {
      ListModel<RowView>::AddOperation(0),
      ListModel<RowView>::AddOperation(1),
      ListModel<RowView>::AddOperation(2)});
    auto connection = scoped_connection(list.connect_operation_signal(
      [&] (const auto& operation) {
        auto update = get<ListModel<RowView>::UpdateOperation>(&operation);
        REQUIRE(update != nullptr);
        REQUIRE(list.get_update().m_row == 1);
        REQUIRE(list.get_update().m_column == 2);
        REQUIRE(any_cast<int>(list.get_update().m_previous) == 1);
        REQUIRE(any_cast<int>(list.get_update().m_value) == 42);
      }));
    table->set(1, 2, 42);
  }
}
