#include <deque>
#include <doctest/doctest.h>
#include <QKeyEvent>
#include <QWidget>
#include "Spire/Ui/KeyObserver.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

TEST_SUITE("KeyObserver") {
  UI_TEST_CASE("key_press") {
    auto window = QWidget();
    auto parent = new QWidget(&window);
    auto child = new QWidget(parent);
    auto observer = KeyObserver(*parent);
    auto targets = std::deque<QWidget*>();
    observer.connect_key_press_signal([&] (auto& target, auto&) {
      targets.push_back(&target);
    });
    press_key(*child, Qt::Key_A);
    REQUIRE(targets.size() == 2);
    REQUIRE(targets.front() == child);
    targets.pop_front();
    REQUIRE(targets.front() == parent);
    targets.pop_front();
    press_key(*parent, Qt::Key_A);
    REQUIRE(targets.size() == 1);
    REQUIRE(targets.front() == parent);
    targets.pop_front();
    release_key(*child, Qt::Key_A);
    press_key(window, Qt::Key_A);
    REQUIRE(targets.empty());
  }

  UI_TEST_CASE("filter") {
    auto window = QWidget();
    auto parent = new QWidget(&window);
    auto child = new QWidget(parent);
    auto observer = KeyObserver(*parent);
    auto targets = std::deque<QWidget*>();
    observer.connect_filtered_key_press_signal([&] (auto& target, auto&) {
      targets.push_back(&target);
      return true;
    });
    press_key(*child, Qt::Key_A);
    REQUIRE(targets.size() == 1);
    REQUIRE(targets.front() == child);
    targets.pop_front();
  }

  UI_TEST_CASE("observer_order_parent_first") {
    auto window = QWidget();
    auto parent = new QWidget(&window);
    auto child = new QWidget(parent);
    auto parent_observer = KeyObserver(*parent);
    auto child_observer = KeyObserver(*child);
    auto targets = std::deque<QWidget*>();
    parent_observer.connect_key_press_signal([&] (auto& target, auto&) {
      targets.push_back(&target);
    });
    child_observer.connect_filtered_key_press_signal([] (auto&, auto&) {
      return true;
    });
    press_key(*child, Qt::Key_A);
    REQUIRE(targets.size() == 1);
    REQUIRE(targets.front() == child);
    targets.pop_front();
  }

  UI_TEST_CASE("observer_order_child_first") {
    auto window = QWidget();
    auto parent = new QWidget(&window);
    auto child = new QWidget(parent);
    auto child_observer = KeyObserver(*child);
    auto parent_observer = KeyObserver(*parent);
    auto targets = std::deque<QWidget*>();
    parent_observer.connect_key_press_signal([&] (auto& target, auto&) {
      targets.push_back(&target);
    });
    child_observer.connect_filtered_key_press_signal([] (auto&, auto&) {
      return true;
    });
    press_key(*child, Qt::Key_A);
    REQUIRE(targets.size() == 2);
    REQUIRE(targets.front() == child);
    targets.pop_front();
    REQUIRE(targets.front() == parent);
    targets.pop_front();
  }

  UI_TEST_CASE("added_and_removed_children") {
    auto window = QWidget();
    auto parent = new QWidget(&window);
    auto observer = KeyObserver(*parent);
    auto targets = std::deque<QWidget*>();
    observer.connect_key_press_signal([&] (auto& target, auto&) {
      targets.push_back(&target);
    });
    auto child = new QWidget(parent);
    press_key(*child, Qt::Key_A);
    REQUIRE(targets.size() == 2);
    REQUIRE(targets.front() == child);
    targets.pop_front();
    REQUIRE(targets.front() == parent);
    targets.pop_front();
    child->setParent(&window);
    press_key(*child, Qt::Key_A);
    REQUIRE(targets.empty());
  }
}
