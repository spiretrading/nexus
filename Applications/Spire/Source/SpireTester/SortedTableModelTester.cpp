#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/SortedTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("SortedTableModel") {
  TEST_CASE("sort") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source,
      {{0, SortedTableModel::Ordering::ASCENDING}},
      [] (const std::any& lhs, const std::any& rhs) {
        return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
      });
    auto order = sorted_model.get_column_order();
    REQUIRE(sorted_model.get<int>(0, 0) == 1);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 9);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    *order.begin() = order.front().cycle();
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<int>(0, 0) == 9);
    REQUIRE(sorted_model.get<int>(1, 0) == 4);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 1);
    REQUIRE(source->get<int>(0, 0) == 4);
    REQUIRE(source->get<int>(1, 0) == 2);
    REQUIRE(source->get<int>(2, 0) == 9);
    REQUIRE(source->get<int>(3, 0) == 1);
    *order.begin() = order.front().cycle();
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 1);
    *order.begin() = order.front().cycle();
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<int>(0, 0) == 1);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 9);
  }

  TEST_CASE("sort_with_wrong_sorting_column") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source,
      {{2, SortedTableModel::Ordering::ASCENDING}},
      [] (const std::any& lhs, const std::any& rhs) {
        return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
      });
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 1);
  }

  TEST_CASE("sort_constructor_without_order_comparator") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source);
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 1);
  }

  TEST_CASE("sort_constructor_without_comparator") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source,
      {{0, SortedTableModel::Ordering::DESCENDING}});
    REQUIRE(sorted_model.get<int>(0, 0) == 9);
    REQUIRE(sorted_model.get<int>(1, 0) == 4);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 1);
  }

  TEST_CASE("sort_constructor_without_order") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source, 
      [] (const std::any& lhs, const std::any& rhs) {
        return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
      });
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 1);
  }

  TEST_CASE("sort_multiple_columns") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    std::vector<SortedTableModel::ColumnOrder> order;
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::DESCENDING});
    order.push_back({2, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Joe");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Jack");
    REQUIRE(sorted_model.get<float>(0, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(1, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(5, 2) == 1.3f);
    order.clear();
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({2, SortedTableModel::Ordering::DESCENDING});
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Joe");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Liam");
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 9);
    REQUIRE(sorted_model.get<int>(4, 0) == 4);
    REQUIRE(sorted_model.get<int>(5, 0) == 6);
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
  }

  TEST_CASE("sort_cycle") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::NONE});
    order.push_back({1, SortedTableModel::Ordering::NONE});
    order.push_back({2, SortedTableModel::Ordering::NONE});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 2);
    REQUIRE(sorted_model.get<int>(4, 0) == 6);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(1, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(2, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(3, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    std::for_each(order.begin(), order.end(),
      [] (SortedTableModel::ColumnOrder& column_order) {
        column_order = column_order.cycle();
      });
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(1, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    std::for_each(order.begin(), order.end(),
      [] (SortedTableModel::ColumnOrder& column_order) {
        column_order = column_order.cycle();
      });
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<int>(0, 0) == 9);
    REQUIRE(sorted_model.get<int>(1, 0) == 9);
    REQUIRE(sorted_model.get<int>(2, 0) == 6);
    REQUIRE(sorted_model.get<int>(3, 0) == 4);
    REQUIRE(sorted_model.get<int>(4, 0) == 2);
    REQUIRE(sorted_model.get<int>(5, 0) == 2);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Joe");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Bob");
    REQUIRE(sorted_model.get<float>(0, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(1, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(2, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(3, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(4, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(5, 2) == 4.1f);
    std::for_each(order.begin(), order.end(),
      [] (SortedTableModel::ColumnOrder& column_order) {
        column_order = column_order.cycle();
      });
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 2);
    REQUIRE(sorted_model.get<int>(4, 0) == 6);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(1, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(2, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(3, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
  }

  TEST_CASE("sort_adjust_order1") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    order.push_back({2, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(1, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    adjust({1, SortedTableModel::Ordering::DESCENDING}, order);
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Joe");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Bob");
    REQUIRE(sorted_model.get<int>(0, 0) == 6);
    REQUIRE(sorted_model.get<int>(1, 0) == 4);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 9);
    REQUIRE(sorted_model.get<int>(4, 0) == 2);
    REQUIRE(sorted_model.get<int>(5, 0) == 2);
    REQUIRE(sorted_model.get<float>(0, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(1, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(2, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(3, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(4, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(5, 2) == 9.2f);
  }

  TEST_CASE("sort_adjust_order2") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    adjust({2, SortedTableModel::Ordering::ASCENDING}, order);
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<float>(0, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(1, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(2, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(5, 2) == 9.2f);
    REQUIRE(sorted_model.get<int>(0, 0) == 9);
    REQUIRE(sorted_model.get<int>(1, 0) == 4);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 2);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Joe");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Bob");
    adjust({0, SortedTableModel::Ordering::DESCENDING}, order);
    sorted_model.set_column_order(order);
    REQUIRE(sorted_model.get<int>(0, 0) == 9);
    REQUIRE(sorted_model.get<int>(1, 0) == 9);
    REQUIRE(sorted_model.get<int>(2, 0) == 6);
    REQUIRE(sorted_model.get<int>(3, 0) == 4);
    REQUIRE(sorted_model.get<int>(4, 0) == 2);
    REQUIRE(sorted_model.get<int>(5, 0) == 2);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Joe");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Bob");
    REQUIRE(sorted_model.get<float>(0, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(1, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(2, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(3, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(4, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(5, 2) == 9.2f);
  }

  TEST_CASE("push") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    auto signal_count = 0;
    auto added_row = 0;
    auto connection = scoped_connection(sorted_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == added_row);
      }));
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    added_row = 2;
    source->push({2, std::string("Tom"), 3.0f});
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 4);
    REQUIRE(sorted_model.get<int>(4, 0) == 6);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<int>(6, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Tom");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(6, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.0f);
    REQUIRE(sorted_model.get<float>(3, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(5, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(6, 2) == 7.8f);
    added_row = 5;
    source->push({7, std::string("Smith"), 4.4f});
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 4);
    REQUIRE(sorted_model.get<int>(4, 0) == 6);
    REQUIRE(sorted_model.get<int>(5, 0) == 7);
    REQUIRE(sorted_model.get<int>(6, 0) == 9);
    REQUIRE(sorted_model.get<int>(7, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Tom");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Smith");
    REQUIRE(sorted_model.get<std::string>(6, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(7, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.0f);
    REQUIRE(sorted_model.get<float>(3, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(5, 2) == 4.4f);
    REQUIRE(sorted_model.get<float>(6, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(7, 2) == 7.8f);
  }

  TEST_CASE("insert") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    auto signal_count = 0;
    auto added_row = 0;
    auto connection = scoped_connection(sorted_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == added_row);
      }));
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    added_row = 2;
    source->insert({2, std::string("Tom"), 3.0f}, 4);
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 4);
    REQUIRE(sorted_model.get<int>(4, 0) == 6);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<int>(6, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Tom");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(6, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.0f);
    REQUIRE(sorted_model.get<float>(3, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(5, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(6, 2) == 7.8f);
    added_row = 5;
    source->insert({7, std::string("Smith"), 4.4f}, 0);
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 4);
    REQUIRE(sorted_model.get<int>(4, 0) == 6);
    REQUIRE(sorted_model.get<int>(5, 0) == 7);
    REQUIRE(sorted_model.get<int>(6, 0) == 9);
    REQUIRE(sorted_model.get<int>(7, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Tom");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Smith");
    REQUIRE(sorted_model.get<std::string>(6, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(7, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.0f);
    REQUIRE(sorted_model.get<float>(3, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(5, 2) == 4.4f);
    REQUIRE(sorted_model.get<float>(6, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(7, 2) == 7.8f);
  }

  TEST_CASE("add_without_order") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::NONE});
    order.push_back({1, SortedTableModel::Ordering::NONE});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    auto signal_count = 0;
    auto added_row = 0;
    auto connection = scoped_connection(sorted_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const TableModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == added_row);
          });
      }));
    added_row = 3;
    source->push({1, std::string("Tom"), 3.0f});
    REQUIRE(signal_count == 1);
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 1);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Tom");
    REQUIRE(sorted_model.get_row_size() == 4);
    added_row = 1;
    source->insert({7, std::string("Smith"), 4.4f}, 1);
    REQUIRE(signal_count == 2);
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 7);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 9);
    REQUIRE(sorted_model.get<int>(4, 0) == 1);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Smith");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Tom");
    REQUIRE(sorted_model.get_row_size() == 5);
    source->remove(2);
    order.clear();
    sorted_model.set_column_order(order);
    added_row = 3;
    source->insert({0, std::string("Liam"), 1.4f}, 3);
    REQUIRE(signal_count == 4);
    REQUIRE(sorted_model.get<int>(0, 0) == 4);
    REQUIRE(sorted_model.get<int>(1, 0) == 7);
    REQUIRE(sorted_model.get<int>(2, 0) == 9);
    REQUIRE(sorted_model.get<int>(3, 0) == 0);
    REQUIRE(sorted_model.get<int>(4, 0) == 1);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Smith");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Tom");
    REQUIRE(sorted_model.get_row_size() == 5);
  }

  TEST_CASE("remove") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    auto signal_count = 0;
    auto removed_row = 0;
    auto connection = scoped_connection(sorted_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto remove_operation = get<TableModel::RemoveOperation>(&operation);
        REQUIRE(remove_operation != nullptr);
        REQUIRE(remove_operation->m_index == removed_row);
      }));
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    removed_row = 4;
    source->remove(2);
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 7.8f);
  }

  TEST_CASE("update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) <
            std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    auto signal_count = 0;
    auto move_count = 0;
    auto update_count = 0;
    auto updated_row = 0;
    auto updated_column = 0;
    auto moved_source = 0;
    auto moved_destination = 0;
    auto connection = scoped_connection(sorted_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        visit(operation,
          [&] (const TableModel::MoveOperation& move_operation) {
            ++move_count;
            REQUIRE(move_operation.m_source == moved_source);
            REQUIRE(move_operation.m_destination == moved_destination);
          },
          [&] (const TableModel::UpdateOperation& update_operation) {
            ++update_count;
            REQUIRE(update_operation.m_row == updated_row);
            REQUIRE(update_operation.m_column == updated_column);
          },
          [] (const auto&) {
            REQUIRE(false);
          });
      }));
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 7.8f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    updated_row = 3;
    updated_column = 2;
    source->set(4, 2, 10.0f);
    REQUIRE(signal_count == 1);
    REQUIRE(move_count == 0);
    REQUIRE(update_count == 1);
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 10.0f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    updated_row = 1;
    updated_column = 1;
    moved_source = 1;
    moved_destination = 0;
    source->set(3, 1, std::string("Ava"));
    REQUIRE(signal_count == 2);
    REQUIRE(move_count == 1);
    REQUIRE(update_count == 2);
    REQUIRE(sorted_model.get<int>(0, 0) == 2);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 6);
    REQUIRE(sorted_model.get<int>(4, 0) == 9);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Ava");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(1, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 10.0f);
    REQUIRE(sorted_model.get<float>(4, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    updated_row = 4;
    updated_column = 0;
    moved_source = 4;
    moved_destination = 0;
    source->set(2, 0, 0);
    REQUIRE(signal_count == 3);
    REQUIRE(move_count == 2);
    REQUIRE(update_count == 3);
    REQUIRE(sorted_model.get<int>(0, 0) == 0);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 4);
    REQUIRE(sorted_model.get<int>(4, 0) == 6);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Ava");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(3, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(4, 2) == 10.0f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
    updated_row = 2;
    updated_column = 1;
    source->set(1, 1, std::string("Tom"));
    REQUIRE(signal_count == 4);
    REQUIRE(move_count == 2);
    REQUIRE(update_count == 4);
    REQUIRE(sorted_model.get<int>(0, 0) == 0);
    REQUIRE(sorted_model.get<int>(1, 0) == 2);
    REQUIRE(sorted_model.get<int>(2, 0) == 2);
    REQUIRE(sorted_model.get<int>(3, 0) == 4);
    REQUIRE(sorted_model.get<int>(4, 0) == 6);
    REQUIRE(sorted_model.get<int>(5, 0) == 9);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Ava");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "Tom");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Liam");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Joe");
    REQUIRE(sorted_model.get<float>(0, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(1, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(3, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(4, 2) == 10.0f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
  }

  TEST_CASE("transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({6, std::string("Liam"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::DESCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    order.push_back({2, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order,
      [] (const std::any& lhs, const std::any& rhs) {
        if(lhs.type() == typeid(int)) {
          return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
        } else if(lhs.type() == typeid(std::string)) {
          return std::any_cast<std::string>(lhs) < std::any_cast<std::string>(rhs);
        } else if(lhs.type() == typeid(float)) {
          return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
        }
        return false;
      });
    auto signal_count = 0;
    auto connection = scoped_connection(sorted_model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
      }));
    source->transact([&] {
      source->push({2, std::string("Ava"), 4.1f});
      source->transact([&] {
        source->set(3, 0, 1);
        source->transact([&] {
          source->insert({6, std::string("Tom"), 11.1f}, 2);
        });
        source->set(5, 1, std::string("Bob"));
      });
    });
    REQUIRE(signal_count == 1);
    REQUIRE(sorted_model.get<int>(0, 0) == 9);
    REQUIRE(sorted_model.get<int>(1, 0) == 6);
    REQUIRE(sorted_model.get<int>(2, 0) == 4);
    REQUIRE(sorted_model.get<int>(3, 0) == 2);
    REQUIRE(sorted_model.get<int>(4, 0) == 2);
    REQUIRE(sorted_model.get<int>(5, 0) == 1);
    REQUIRE(sorted_model.get<std::string>(0, 1) == "Jack");
    REQUIRE(sorted_model.get<std::string>(1, 1) == "Tom");
    REQUIRE(sorted_model.get<std::string>(2, 1) == "John");
    REQUIRE(sorted_model.get<std::string>(3, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(4, 1) == "Bob");
    REQUIRE(sorted_model.get<std::string>(5, 1) == "Liam");
    REQUIRE(sorted_model.get<float>(0, 2) == 1.3f);
    REQUIRE(sorted_model.get<float>(1, 2) == 11.1f);
    REQUIRE(sorted_model.get<float>(2, 2) == 3.7f);
    REQUIRE(sorted_model.get<float>(3, 2) == 4.1f);
    REQUIRE(sorted_model.get<float>(4, 2) == 9.2f);
    REQUIRE(sorted_model.get<float>(5, 2) == 7.8f);
  }
}
