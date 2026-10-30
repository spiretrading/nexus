#include <deque>
#include <utility>
#include <doctest/doctest.h>
#include <QMouseEvent>
#include <QWidget>
#include "Spire/Ui/MouseObserver.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

TEST_SUITE("MouseObserver") {
  TEST_CASE("mouse_events") {
    run_ui_test([] {
      auto window = QWidget();
      auto parent = new QWidget(&window);
      auto child = new QWidget(parent);
      auto observer = MouseObserver(*parent);
      auto events = std::deque<std::pair<QWidget*, QEvent::Type>>();
      observer.connect_mouse_signal([&] (auto& target, auto& event) {
        events.push_back(std::pair(&target, event.type()));
      });
      press_mouse(*child, Qt::LeftButton);
      REQUIRE(events.size() == 2);
      REQUIRE(events.front() == std::pair(child, QEvent::MouseButtonPress));
      events.pop_front();
      REQUIRE(events.front() == std::pair(parent, QEvent::MouseButtonPress));
      events.pop_front();
      release_mouse(*child, Qt::LeftButton);
      REQUIRE(events.size() == 2);
      REQUIRE(events.front() == std::pair(child, QEvent::MouseButtonRelease));
      events.pop_front();
      REQUIRE(events.front() == std::pair(parent, QEvent::MouseButtonRelease));
      events.pop_front();
      press_mouse(window, Qt::LeftButton);
      release_mouse(window, Qt::LeftButton);
      REQUIRE(events.empty());
    });
  }

  TEST_CASE("filter") {
    run_ui_test([] {
      auto window = QWidget();
      auto parent = new QWidget(&window);
      auto child = new QWidget(parent);
      auto observer = MouseObserver(*parent);
      auto targets = std::deque<QWidget*>();
      observer.connect_filtered_mouse_signal([&] (auto& target, auto&) {
        targets.push_back(&target);
        return true;
      });
      press_mouse(*child, Qt::LeftButton);
      REQUIRE(targets.size() == 1);
      REQUIRE(targets.front() == child);
      targets.pop_front();
    });
  }

  TEST_CASE("observer_order") {
    run_ui_test([] {
      auto window = QWidget();
      auto parent = new QWidget(&window);
      auto child = new QWidget(parent);
      auto parent_observer = MouseObserver(*parent);
      auto child_observer = MouseObserver(*child);
      auto targets = std::deque<QWidget*>();
      parent_observer.connect_mouse_signal([&] (auto& target, auto&) {
        targets.push_back(&target);
      });
      child_observer.connect_filtered_mouse_signal([] (auto&, auto&) {
        return true;
      });
      press_mouse(*child, Qt::LeftButton);
      REQUIRE(targets.size() == 1);
      REQUIRE(targets.front() == child);
      targets.pop_front();
    });
    run_ui_test([] {
      auto window = QWidget();
      auto parent = new QWidget(&window);
      auto child = new QWidget(parent);
      auto child_observer = MouseObserver(*child);
      auto parent_observer = MouseObserver(*parent);
      auto targets = std::deque<QWidget*>();
      parent_observer.connect_mouse_signal([&] (auto& target, auto&) {
        targets.push_back(&target);
      });
      child_observer.connect_filtered_mouse_signal([] (auto&, auto&) {
        return true;
      });
      press_mouse(*child, Qt::LeftButton);
      REQUIRE(targets.empty());
    });
  }

  TEST_CASE("added_and_removed_children") {
    run_ui_test([] {
      auto window = QWidget();
      auto parent = new QWidget(&window);
      auto observer = MouseObserver(*parent);
      auto targets = std::deque<QWidget*>();
      observer.connect_mouse_signal([&] (auto& target, auto&) {
        targets.push_back(&target);
      });
      auto child = new QWidget(parent);
      press_mouse(*child, Qt::LeftButton);
      REQUIRE(targets.size() == 2);
      REQUIRE(targets.front() == child);
      targets.pop_front();
      REQUIRE(targets.front() == parent);
      targets.pop_front();
      child->setParent(&window);
      press_mouse(*child, Qt::LeftButton);
      REQUIRE(targets.empty());
    });
  }
}
