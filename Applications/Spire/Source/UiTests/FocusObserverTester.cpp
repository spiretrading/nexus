#include <deque>
#include <memory>
#include <utility>
#include <boost/lexical_cast.hpp>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Tests;

using State = FocusObserver::State;

namespace {
  void activate(QWidget& window) {
    window.show();
    window.activateWindow();
    QApplication::processEvents();
    REQUIRE(window.isActiveWindow());
  }
}

TEST_SUITE("FocusObserver") {
  UI_TEST_CASE("focus") {
    auto window = QWidget();
    auto parent = new QWidget(&window);
    auto child = new QWidget(parent);
    auto sibling = new QWidget(&window);
    activate(window);
    auto parent_observer = FocusObserver(*parent);
    auto child_observer = FocusObserver(*child);
    auto updates = std::deque<std::pair<QWidget*, State>>();
    parent_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(parent, state));
    });
    child_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(child, state));
    });
    REQUIRE(parent_observer.get_state() == State::NONE);
    REQUIRE(child_observer.get_state() == State::NONE);
    child->setFocus(Qt::MouseFocusReason);
    REQUIRE(updates.size() == 2);
    REQUIRE(updates.front() == std::pair(parent, State::FOCUS_IN));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(child, State::FOCUS));
    updates.pop_front();
    REQUIRE(parent_observer.get_state() == State::FOCUS_IN);
    REQUIRE(child_observer.get_state() == State::FOCUS);
    parent->setFocus(Qt::MouseFocusReason);
    REQUIRE(updates.size() == 2);
    REQUIRE(updates.front() == std::pair(parent, State::FOCUS));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(child, State::NONE));
    updates.pop_front();
    sibling->setFocus(Qt::MouseFocusReason);
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == std::pair(parent, State::NONE));
    updates.pop_front();
    REQUIRE(parent_observer.get_state() == State::NONE);
    REQUIRE(child_observer.get_state() == State::NONE);
  }

  UI_TEST_CASE("focus_visible") {
    auto window = QWidget();
    auto first = new QWidget(&window);
    auto second = new QWidget(&window);
    auto third = new QWidget(&window);
    activate(window);
    auto first_observer = FocusObserver(*first);
    auto second_observer = FocusObserver(*second);
    auto third_observer = FocusObserver(*third);
    first->setFocus(Qt::TabFocusReason);
    REQUIRE(first_observer.get_state() == State::FOCUS_VISIBLE);
    second->setFocus(Qt::OtherFocusReason);
    REQUIRE(first_observer.get_state() == State::NONE);
    REQUIRE(second_observer.get_state() == State::FOCUS_VISIBLE);
    third->setFocus(Qt::MouseFocusReason);
    REQUIRE(second_observer.get_state() == State::NONE);
    REQUIRE(third_observer.get_state() == State::FOCUS);
    first->setFocus(Qt::OtherFocusReason);
    REQUIRE(first_observer.get_state() == State::FOCUS);
    second->setFocus(Qt::BacktabFocusReason);
    REQUIRE(second_observer.get_state() == State::FOCUS_VISIBLE);
    third->setFocus(Qt::ShortcutFocusReason);
    REQUIRE(third_observer.get_state() == State::FOCUS_VISIBLE);
  }

  UI_TEST_CASE("initial_state") {
    auto window = QWidget();
    auto parent = new QWidget(&window);
    auto child = new QWidget(parent);
    activate(window);
    child->setFocus(Qt::MouseFocusReason);
    auto parent_observer = FocusObserver(*parent);
    auto child_observer = FocusObserver(*child);
    REQUIRE(parent_observer.get_state() == State::FOCUS_IN);
    REQUIRE(child_observer.get_state() == State::FOCUS);
    REQUIRE(find_focus_state(window) == State::FOCUS_IN);
    REQUIRE(find_focus_state(*child) == State::FOCUS);
    window.hide();
    REQUIRE(find_focus_state(*child) == State::NONE);
  }

  UI_TEST_CASE("shared_observers") {
    auto window = QWidget();
    auto widget = new QWidget(&window);
    auto other = new QWidget(&window);
    activate(window);
    auto first_observer = FocusObserver(*widget);
    auto first_updates = std::deque<State>();
    first_observer.connect_state_signal([&] (auto state) {
      first_updates.push_back(state);
    });
    auto second_observer = std::make_unique<FocusObserver>(*widget);
    auto second_updates = std::deque<State>();
    auto second_connection =
      second_observer->connect_state_signal([&] (auto state) {
        second_updates.push_back(state);
      });
    widget->setFocus(Qt::MouseFocusReason);
    REQUIRE(first_updates.size() == 1);
    REQUIRE(first_updates.front() == State::FOCUS);
    first_updates.pop_front();
    REQUIRE(second_updates.size() == 1);
    REQUIRE(second_updates.front() == State::FOCUS);
    second_updates.pop_front();
    second_observer.reset();
    REQUIRE(!second_connection.connected());
    other->setFocus(Qt::MouseFocusReason);
    REQUIRE(first_updates.size() == 1);
    REQUIRE(first_updates.front() == State::NONE);
    first_updates.pop_front();
    REQUIRE(second_updates.empty());
  }

  UI_TEST_CASE("observer_order") {
    auto window = QWidget();
    auto widget = new QWidget(&window);
    auto other = new QWidget(&window);
    activate(window);
    auto first_observer = FocusObserver(*widget);
    auto second_observer = FocusObserver(*widget);
    auto updates = std::deque<std::pair<int, State>>();
    second_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(2, state));
    });
    first_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(1, state));
    });
    second_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(3, state));
    });
    widget->setFocus(Qt::MouseFocusReason);
    REQUIRE(updates.size() == 3);
    REQUIRE(updates.front() == std::pair(1, State::FOCUS));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(2, State::FOCUS));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(3, State::FOCUS));
    updates.pop_front();
    auto third_observer = FocusObserver(*widget);
    third_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(4, state));
    });
    other->setFocus(Qt::MouseFocusReason);
    REQUIRE(updates.size() == 4);
    REQUIRE(updates.front() == std::pair(1, State::NONE));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(2, State::NONE));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(3, State::NONE));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(4, State::NONE));
    updates.pop_front();
  }

  UI_TEST_CASE("added_and_removed_observers") {
    auto window = QWidget();
    auto first = new QWidget(&window);
    auto second = new QWidget(&window);
    auto third = new QWidget(&window);
    activate(window);
    auto first_observer = std::make_unique<FocusObserver>(*first);
    auto second_observer = std::make_unique<FocusObserver>(*second);
    auto third_observer = std::make_unique<FocusObserver>(*third);
    second_observer.reset();
    third->setFocus(Qt::MouseFocusReason);
    REQUIRE(first_observer->get_state() == State::NONE);
    REQUIRE(third_observer->get_state() == State::FOCUS);
    first_observer.reset();
    second_observer = std::make_unique<FocusObserver>(*second);
    second->setFocus(Qt::MouseFocusReason);
    REQUIRE(second_observer->get_state() == State::FOCUS);
    REQUIRE(third_observer->get_state() == State::NONE);
    first_observer = std::make_unique<FocusObserver>(*first);
    first->setFocus(Qt::MouseFocusReason);
    REQUIRE(first_observer->get_state() == State::FOCUS);
    REQUIRE(second_observer->get_state() == State::NONE);
    REQUIRE(third_observer->get_state() == State::NONE);
  }

  UI_TEST_CASE("nested_focus_change") {
    auto window = QWidget();
    auto first = new QWidget(&window);
    auto parent = new QWidget(&window);
    auto child = new QWidget(parent);
    activate(window);
    auto first_observer = FocusObserver(*first);
    auto child_observer = FocusObserver(*child);
    auto parent_observer = std::make_unique<FocusObserver>(*parent);
    auto updates = std::deque<std::pair<QWidget*, State>>();
    first_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(first, state));
    });
    child_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(child, state));
      if(state == State::FOCUS) {
        parent_observer.reset();
        first->setFocus(Qt::MouseFocusReason);
      }
    });
    parent_observer->connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(parent, state));
    });
    child->setFocus(Qt::MouseFocusReason);
    REQUIRE(updates.size() == 3);
    REQUIRE(updates.front() == std::pair(child, State::FOCUS));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(first, State::FOCUS));
    updates.pop_front();
    REQUIRE(updates.front() == std::pair(child, State::NONE));
    updates.pop_front();
    REQUIRE(first_observer.get_state() == State::FOCUS);
    REQUIRE(child_observer.get_state() == State::NONE);
  }

  UI_TEST_CASE("delete_during_signal") {
    auto window = QWidget();
    auto first = new QWidget(&window);
    auto second = new QWidget(&window);
    activate(window);
    first->setFocus(Qt::MouseFocusReason);
    auto first_observer = FocusObserver(*first);
    auto second_observer = std::make_unique<FocusObserver>(*second);
    auto updates = std::deque<std::pair<QWidget*, State>>();
    first_observer.connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(first, state));
      second_observer.reset();
    });
    second_observer->connect_state_signal([&] (auto state) {
      updates.push_back(std::pair(second, state));
    });
    second->setFocus(Qt::MouseFocusReason);
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == std::pair(first, State::NONE));
    updates.pop_front();
    auto third_observer = FocusObserver(*second);
    REQUIRE(third_observer.get_state() == State::FOCUS);
  }

  TEST_CASE("is_set") {
    REQUIRE(!is_set(State::NONE, State::FOCUS_IN));
    REQUIRE(is_set(State::FOCUS_IN, State::FOCUS_IN));
    REQUIRE(is_set(State::FOCUS, State::FOCUS_IN));
    REQUIRE(is_set(State::FOCUS_VISIBLE, State::FOCUS));
  }

  TEST_CASE("stream") {
    REQUIRE(lexical_cast<std::string>(State::NONE) == "NONE");
    REQUIRE(lexical_cast<std::string>(State::FOCUS_IN) == "FOCUS_IN");
    REQUIRE(lexical_cast<std::string>(State::FOCUS) == "FOCUS");
    REQUIRE(
      lexical_cast<std::string>(State::FOCUS_VISIBLE) == "FOCUS_VISIBLE");
  }
}
