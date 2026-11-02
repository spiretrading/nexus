#include <deque>
#include <memory>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Ui/GlobalPositionObserver.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

namespace {
  QPoint get_global_position(const QWidget& widget) {
    return widget.mapToGlobal(QPoint(0, 0));
  }
}

TEST_SUITE("GlobalPositionObserver") {
  UI_TEST_CASE("moves") {
    auto window = QWidget();
    window.setGeometry(100, 100, 300, 300);
    auto parent = new QWidget(&window);
    parent->setGeometry(10, 10, 200, 200);
    auto child = new QWidget(parent);
    child->setGeometry(20, 20, 50, 50);
    window.show();
    QApplication::processEvents();
    auto observer = GlobalPositionObserver(*child);
    REQUIRE(observer.get_position() == get_global_position(*child));
    auto positions = std::deque<QPoint>();
    observer.connect_position_signal([&] (auto position) {
      positions.push_back(position);
    });
    child->move(30, 40);
    QApplication::processEvents();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front() == get_global_position(*child));
    positions.pop_front();
    parent->move(15, 25);
    QApplication::processEvents();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front() == get_global_position(*child));
    positions.pop_front();
    window.move(150, 160);
    QApplication::processEvents();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front() == get_global_position(*child));
    positions.pop_front();
    child->move(30, 40);
    parent->move(15, 25);
    QApplication::processEvents();
    REQUIRE(positions.empty());
    REQUIRE(observer.get_position() == get_global_position(*child));
  }

  UI_TEST_CASE("reparent") {
    auto window = QWidget();
    window.setGeometry(100, 100, 300, 300);
    auto first_parent = new QWidget(&window);
    first_parent->setGeometry(10, 10, 100, 100);
    auto second_parent = new QWidget(&window);
    second_parent->setGeometry(150, 150, 100, 100);
    auto child = new QWidget(first_parent);
    child->setGeometry(5, 5, 20, 20);
    window.show();
    QApplication::processEvents();
    auto observer = GlobalPositionObserver(*child);
    child->setParent(second_parent);
    child->move(5, 5);
    child->show();
    QApplication::processEvents();
    REQUIRE(observer.get_position() == get_global_position(*child));
    auto positions = std::deque<QPoint>();
    observer.connect_position_signal([&] (auto position) {
      positions.push_back(position);
    });
    first_parent->move(20, 20);
    QApplication::processEvents();
    REQUIRE(positions.empty());
    second_parent->move(160, 140);
    QApplication::processEvents();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front() == get_global_position(*child));
    positions.pop_front();
  }

  UI_TEST_CASE("observer_order") {
    auto window = QWidget();
    window.setGeometry(100, 100, 300, 300);
    auto parent = new QWidget(&window);
    parent->setGeometry(10, 10, 200, 200);
    auto first_child = new QWidget(parent);
    first_child->setGeometry(20, 20, 50, 50);
    auto second_child = new QWidget(parent);
    second_child->setGeometry(80, 80, 50, 50);
    window.show();
    QApplication::processEvents();
    auto first_child_observer = GlobalPositionObserver(*first_child);
    auto parent_observer = GlobalPositionObserver(*parent);
    auto second_child_observer = GlobalPositionObserver(*second_child);
    auto targets = std::deque<QWidget*>();
    first_child_observer.connect_position_signal([&] (auto) {
      targets.push_back(first_child);
    });
    parent_observer.connect_position_signal([&] (auto) {
      targets.push_back(parent);
    });
    second_child_observer.connect_position_signal([&] (auto) {
      targets.push_back(second_child);
    });
    parent->move(15, 25);
    QApplication::processEvents();
    REQUIRE(targets.size() == 3);
    REQUIRE(targets.front() == first_child);
    targets.pop_front();
    REQUIRE(targets.front() == parent);
    targets.pop_front();
    REQUIRE(targets.front() == second_child);
    targets.pop_front();
  }

  UI_TEST_CASE("observers_changed_during_move") {
    auto window = QWidget();
    window.setGeometry(100, 100, 300, 300);
    auto child = new QWidget(&window);
    child->setGeometry(10, 10, 50, 50);
    window.show();
    QApplication::processEvents();
    auto first_observer = GlobalPositionObserver(*child);
    auto second_observer = std::make_unique<GlobalPositionObserver>(*child);
    auto third_observer = std::unique_ptr<GlobalPositionObserver>();
    auto calls = std::deque<int>();
    first_observer.connect_position_signal([&] (auto) {
      calls.push_back(1);
      second_observer = nullptr;
      if(!third_observer) {
        third_observer = std::make_unique<GlobalPositionObserver>(*child);
        third_observer->connect_position_signal([&] (auto) {
          calls.push_back(3);
        });
      }
    });
    second_observer->connect_position_signal([&] (auto) {
      calls.push_back(2);
    });
    child->move(20, 20);
    QApplication::processEvents();
    REQUIRE(calls.size() == 1);
    REQUIRE(calls.front() == 1);
    calls.pop_front();
    child->move(30, 30);
    QApplication::processEvents();
    REQUIRE(calls.size() == 2);
    REQUIRE(calls.front() == 1);
    calls.pop_front();
    REQUIRE(calls.front() == 3);
    calls.pop_front();
  }
}
