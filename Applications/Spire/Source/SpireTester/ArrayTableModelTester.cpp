#include <boost/variant/get.hpp>
#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("ArrayTableModel") {
  TEST_CASE("push") {
    auto model = ArrayTableModel();
    auto received_index = -1;
    model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        if(auto add_operation = get<TableModel::AddOperation>(&operation)) {
          received_index = add_operation->m_index;
        } else {
          REQUIRE(false);
        }
      });
    REQUIRE(model.get_row_size() == 0);
    REQUIRE(model.get_column_size() == 0);
    REQUIRE_NOTHROW(model.push({3, 1, 4}));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(std::any_cast<int>(model.get(0, 0)) == 3);
    REQUIRE(std::any_cast<int>(model.get(0, 1)) == 1);
    REQUIRE(std::any_cast<int>(model.get(0, 2)) == 4);
    REQUIRE(received_index == 0);
    received_index = -1;
    REQUIRE_THROWS(model.push({2, 7}));
    REQUIRE(received_index == -1);
    REQUIRE_NOTHROW(model.push({2, 7, 1}));
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(std::any_cast<int>(model.get(1, 0)) == 2);
    REQUIRE(std::any_cast<int>(model.get(1, 1)) == 7);
    REQUIRE(std::any_cast<int>(model.get(1, 2)) == 1);
    REQUIRE(received_index == 1);
  }
}
