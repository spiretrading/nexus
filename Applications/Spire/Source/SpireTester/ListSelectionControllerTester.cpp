#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/ListSelectionController.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  bool find_value(ListModel<int>& list, int value) {
    return std::find(list.begin(), list.end(), value) != list.end();
  }
}

TEST_SUITE("ListSelectionController") {
  TEST_CASE("single_click") {
    auto selection = std::make_shared<ArrayListModel<int>>();
    auto operations = std::deque<ListModel<int>::Operation>();
    auto connection = scoped_connection(selection->connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        operations.push_back(operation);
      }));
    auto controller = ListSelectionController(selection, 0);
    REQUIRE(controller.get_mode() == ListSelectionController::Mode::SINGLE);
    controller.click(5);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 5);
    controller.click(3);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 3);
    operations.clear();
    controller.click(3);
    REQUIRE(operations.empty());
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 3);
    selection->remove(0);
    controller.click(3);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 3);
  }

  TEST_CASE("single_click_existing_selection") {
    auto selection = std::make_shared<ArrayListModel<int>>();
    selection->push(1);
    selection->push(2);
    selection->push(3);
    selection->push(4);
    auto controller = ListSelectionController(selection, 0);
    REQUIRE(controller.get_mode() == ListSelectionController::Mode::SINGLE);
    controller.click(2);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 2);
  }

  TEST_CASE("single_navigate") {
    auto selection = std::make_shared<ArrayListModel<int>>();
    auto controller = ListSelectionController(selection, 0);
    auto operations = std::deque<ListModel<int>::Operation>();
    auto connection = scoped_connection(controller.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        operations.push_back(operation);
      }));
    controller.navigate(5);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 5);
    controller.navigate(3);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 3);
    operations.clear();
    controller.navigate(3);
    REQUIRE(operations.empty());
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 3);
    selection->remove(0);
    controller.navigate(3);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 3);
  }

  TEST_CASE("incremental_click") {
    auto selection = std::make_shared<ArrayListModel<int>>();
    auto controller = ListSelectionController(selection, 0);
    controller.set_mode(ListSelectionController::Mode::INCREMENTAL);
    controller.click(5);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 5);
    controller.click(6);
    REQUIRE(selection->get_size() == 2);
    REQUIRE(find_value(*selection, 5));
    REQUIRE(find_value(*selection, 6));
    controller.click(2);
    REQUIRE(selection->get_size() == 3);
    REQUIRE(find_value(*selection, 5));
    REQUIRE(find_value(*selection, 6));
    REQUIRE(find_value(*selection, 2));
    controller.click(5);
    REQUIRE(selection->get_size() == 2);
    REQUIRE(find_value(*selection, 6));
    REQUIRE(find_value(*selection, 2));
    controller.click(5);
    REQUIRE(selection->get_size() == 3);
    REQUIRE(find_value(*selection, 5));
    controller.click(5);
    controller.click(6);
    controller.click(2);
    REQUIRE(selection->get_size() == 0);
  }

  TEST_CASE("incremental_navigate") {
    auto selection = std::make_shared<ArrayListModel<int>>();
    auto controller = ListSelectionController(selection, 0);
    controller.set_mode(ListSelectionController::Mode::INCREMENTAL);
    controller.navigate(5);
    REQUIRE(selection->get_size() == 0);
    controller.navigate(6);
    REQUIRE(selection->get_size() == 0);
    controller.navigate(5);
    REQUIRE(selection->get_size() == 0);
    controller.navigate(5);
    REQUIRE(selection->get_size() == 0);
    controller.click(5);
    controller.navigate(5);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(find_value(*selection, 5));
  }

  TEST_CASE("range_click") {
    auto selection = std::make_shared<ArrayListModel<int>>();
    auto controller = ListSelectionController(selection, 0);
    controller.set_mode(ListSelectionController::Mode::RANGE);
    controller.click(3);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(find_value(*selection, 3));
    controller.click(6);
    REQUIRE(selection->get_size() == 4);
    REQUIRE(find_value(*selection, 3));
    REQUIRE(find_value(*selection, 4));
    REQUIRE(find_value(*selection, 5));
    REQUIRE(find_value(*selection, 6));
    controller.click(2);
    REQUIRE(selection->get_size() == 2);
    REQUIRE(find_value(*selection, 2));
    REQUIRE(find_value(*selection, 3));
  }

  TEST_CASE("range_navigation_single_selection") {
    auto selection = std::make_shared<ListSingleSelectionModel>();
    auto controller = ListSelectionController(selection, 0);
    controller.set_mode(ListSelectionController::Mode::RANGE);
    controller.add(0);
    controller.add(1);
    controller.add(2);
    controller.navigate(0);
    controller.navigate(1);
    controller.navigate(2);
    controller.navigate(1);
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 1);
  }

  TEST_CASE("range_reentrant") { 
    auto selection = std::make_shared<ListSingleSelectionModel>();
    auto controller = ListSelectionController(selection, 0);
    controller.set_mode(ListSelectionController::Mode::RANGE);
    controller.add(0); 
    controller.add(1); 
    controller.add(2); 
    controller.navigate(0); 
    controller.navigate(2); 
    REQUIRE(selection->get_size() == 1);
    REQUIRE(selection->get(0) == 2);
  }
}
