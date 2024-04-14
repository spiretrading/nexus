#include <doctest/doctest.h>
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TableSelectionController.hpp"

using namespace Spire;

TEST_SUITE("TableSelectionController") {
  TEST_CASE("back_range") {
    const auto ROWS = 4;
    const auto COLUMNS = 6;
    auto controller = TableSelectionController(
      std::make_shared<TableSelectionModel>(
        std::make_shared<TableEmptySelectionModel>(),
        std::make_shared<ListSingleSelectionModel>(),
        std::make_shared<ListEmptySelectionModel>()), ROWS, COLUMNS);
    controller.click(TableIndex(3, 0));
    REQUIRE(controller.get_selection()->get_item_selection()->get_size() == 0);
    REQUIRE(controller.get_selection()->get_row_selection()->get_size() == 1);
    REQUIRE(controller.get_selection()->get_row_selection()->get(0) == 3);
    REQUIRE(
      controller.get_selection()->get_column_selection()->get_size() == 0);
    controller.set_mode(TableSelectionController::Mode::RANGE);
    controller.click(TableIndex(2, 4));
    REQUIRE(controller.get_selection()->get_item_selection()->get_size() == 0);
    REQUIRE(controller.get_selection()->get_row_selection()->get_size() == 1);
    REQUIRE(controller.get_selection()->get_row_selection()->get(0) == 2);
    REQUIRE(
      controller.get_selection()->get_column_selection()->get_size() == 0);
  }
}
