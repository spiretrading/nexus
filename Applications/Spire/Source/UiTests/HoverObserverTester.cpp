#include <deque>
#include <memory>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Ui/HoverObserver.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

namespace {
  using State = HoverObserver::State;

  struct Tree {
    std::unique_ptr<QWidget> m_window;
    QWidget* m_parent;
    QWidget* m_child;
  };

  Tree make_tree() {
    auto window = std::make_unique<QWidget>();
    window->setGeometry(100, 100, 200, 200);
    auto parent = new QWidget(window.get());
    parent->setGeometry(10, 10, 100, 100);
    auto child = new QWidget(parent);
    child->setGeometry(20, 20, 40, 40);
    window->show();
    QApplication::processEvents();
    return Tree(std::move(window), parent, child);
  }
}

TEST_SUITE("HoverObserver") {
  UI_TEST_CASE("hover") {
    auto tree = make_tree();
    move_cursor(*tree.m_window, QPoint(150, 150));
    auto observer = HoverObserver(*tree.m_parent);
    auto states = std::deque<State>();
    observer.connect_state_signal([&] (auto state) {
      states.push_back(state);
    });
    REQUIRE(observer.get_state() == State::NONE);
    move_cursor(*tree.m_parent, QPoint(5, 5));
    REQUIRE(states.size() == 1);
    REQUIRE(states.front() == State::MOUSE_IN);
    states.pop_front();
    move_cursor(*tree.m_parent, QPoint(30, 30));
    REQUIRE(states.size() == 1);
    REQUIRE(states.front() == State::MOUSE_OVER);
    states.pop_front();
    move_cursor(*tree.m_parent, QPoint(5, 5));
    REQUIRE(states.size() == 1);
    REQUIRE(states.front() == State::MOUSE_IN);
    states.pop_front();
    move_cursor(*tree.m_window, QPoint(150, 150));
    REQUIRE(states.size() == 1);
    REQUIRE(states.front() == State::NONE);
    states.pop_front();
  }

  UI_TEST_CASE("nested_observers") {
    auto tree = make_tree();
    auto window_observer = HoverObserver(*tree.m_window);
    auto parent_observer = HoverObserver(*tree.m_parent);
    auto child_observer = HoverObserver(*tree.m_child);
    auto shared_observer = HoverObserver(*tree.m_parent);
    move_cursor(*tree.m_parent, QPoint(30, 30));
    REQUIRE(window_observer.get_state() == State::MOUSE_OVER);
    REQUIRE(parent_observer.get_state() == State::MOUSE_OVER);
    REQUIRE(child_observer.get_state() == State::MOUSE_IN);
    REQUIRE(shared_observer.get_state() == State::MOUSE_OVER);
    move_cursor(*tree.m_parent, QPoint(5, 5));
    REQUIRE(window_observer.get_state() == State::MOUSE_OVER);
    REQUIRE(parent_observer.get_state() == State::MOUSE_IN);
    REQUIRE(child_observer.get_state() == State::NONE);
    REQUIRE(shared_observer.get_state() == State::MOUSE_IN);
    auto late_observer = HoverObserver(*tree.m_parent);
    REQUIRE(late_observer.get_state() == State::MOUSE_IN);
  }

  UI_TEST_CASE("added_and_removed_children") {
    auto tree = make_tree();
    auto observer = HoverObserver(*tree.m_parent);
    move_cursor(*tree.m_parent, QPoint(5, 5));
    REQUIRE(observer.get_state() == State::MOUSE_IN);
    auto added = new QWidget(tree.m_parent);
    added->setGeometry(0, 0, 15, 15);
    added->show();
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_OVER);
    delete added;
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_IN);
    auto subtree = new QWidget(tree.m_parent);
    subtree->setGeometry(0, 0, 15, 15);
    auto leaf = new QWidget(subtree);
    leaf->setGeometry(0, 0, 15, 15);
    subtree->show();
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_OVER);
    delete subtree;
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_IN);
    auto replacement = new QWidget(tree.m_parent);
    replacement->setGeometry(0, 0, 15, 15);
    replacement->show();
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_OVER);
  }

  UI_TEST_CASE("visibility_and_enabled") {
    auto tree = make_tree();
    auto observer = HoverObserver(*tree.m_parent);
    move_cursor(*tree.m_parent, QPoint(30, 30));
    REQUIRE(observer.get_state() == State::MOUSE_OVER);
    tree.m_child->hide();
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_IN);
    tree.m_child->show();
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_OVER);
    tree.m_parent->setEnabled(false);
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::NONE);
    tree.m_parent->setEnabled(true);
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_OVER);
    tree.m_parent->hide();
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::NONE);
  }

  UI_TEST_CASE("moved_under_cursor") {
    auto tree = make_tree();
    auto observer = HoverObserver(*tree.m_parent);
    move_cursor(*tree.m_parent, QPoint(70, 70));
    REQUIRE(observer.get_state() == State::MOUSE_IN);
    tree.m_child->move(50, 50);
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::MOUSE_OVER);
    tree.m_parent->move(150, 150);
    QApplication::processEvents();
    REQUIRE(observer.get_state() == State::NONE);
  }
}
