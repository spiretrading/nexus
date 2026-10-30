#include <deque>
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
  TEST_CASE("moves") {
    run_ui_test([] {
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
    });
  }

  TEST_CASE("reparent") {
    run_ui_test([] {
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
    });
  }
}
