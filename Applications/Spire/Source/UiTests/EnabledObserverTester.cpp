#include <deque>
#include <utility>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Ui/EnabledObserver.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

TEST_SUITE("EnabledObserver") {
  TEST_CASE("enabled_change") {
    run_ui_test([] {
      auto window = QWidget();
      auto parent = new QWidget(&window);
      auto child = new QWidget(parent);
      auto observer = EnabledObserver(*child);
      auto updates = std::deque<bool>();
      observer.connect_enabled_signal([&] (auto is_enabled) {
        updates.push_back(is_enabled);
      });
      child->setEnabled(false);
      REQUIRE(updates.size() == 1);
      REQUIRE(!updates.front());
      updates.pop_front();
      child->setEnabled(true);
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.front());
      updates.pop_front();
      window.setEnabled(false);
      REQUIRE(updates.size() == 1);
      REQUIRE(!updates.front());
      updates.pop_front();
      parent->setEnabled(false);
      REQUIRE(updates.empty());
      window.setEnabled(true);
      REQUIRE(updates.empty());
      parent->setEnabled(true);
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.front());
      updates.pop_front();
      child->setEnabled(false);
      REQUIRE(updates.size() == 1);
      REQUIRE(!updates.front());
      updates.pop_front();
      parent->setEnabled(false);
      parent->setEnabled(true);
      REQUIRE(updates.empty());
    });
  }

  TEST_CASE("reparent") {
    run_ui_test([] {
      auto window = QWidget();
      auto enabled_parent = new QWidget(&window);
      auto disabled_parent = new QWidget(&window);
      disabled_parent->setEnabled(false);
      auto child = new QWidget(enabled_parent);
      auto observer = EnabledObserver(*child);
      auto updates = std::deque<bool>();
      observer.connect_enabled_signal([&] (auto is_enabled) {
        updates.push_back(is_enabled);
      });
      child->setParent(disabled_parent);
      REQUIRE(updates.size() == 1);
      REQUIRE(!updates.front());
      updates.pop_front();
      disabled_parent->setEnabled(true);
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.front());
      updates.pop_front();
      enabled_parent->setEnabled(false);
      enabled_parent->setEnabled(true);
      REQUIRE(updates.empty());
      disabled_parent->setEnabled(false);
      REQUIRE(updates.size() == 1);
      REQUIRE(!updates.front());
      updates.pop_front();
      child->setParent(enabled_parent);
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.front());
      updates.pop_front();
      enabled_parent->setEnabled(false);
      REQUIRE(updates.size() == 1);
      REQUIRE(!updates.front());
      updates.pop_front();
      child->setParent(nullptr);
      REQUIRE(updates.empty());
      child->setEnabled(true);
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.front());
      updates.pop_front();
      child->setParent(enabled_parent);
      REQUIRE(updates.size() == 1);
      REQUIRE(!updates.front());
      updates.pop_front();
    });
  }

  TEST_CASE("observer_order") {
    run_ui_test([] {
      auto window = QWidget();
      auto parent = new QWidget(&window);
      auto child = new QWidget(parent);
      auto parent_observer = EnabledObserver(*parent);
      auto child_observer = EnabledObserver(*child);
      auto updates = std::deque<std::pair<QWidget*, bool>>();
      parent_observer.connect_enabled_signal([&] (auto is_enabled) {
        updates.push_back(std::pair(parent, is_enabled));
      });
      child_observer.connect_enabled_signal([&] (auto is_enabled) {
        updates.push_back(std::pair(child, is_enabled));
      });
      parent->setEnabled(false);
      REQUIRE(updates.size() == 2);
      REQUIRE(updates.front() == std::pair(child, false));
      updates.pop_front();
      REQUIRE(updates.front() == std::pair(parent, false));
      updates.pop_front();
      parent->setEnabled(true);
      REQUIRE(updates.size() == 2);
      REQUIRE(updates.front() == std::pair(child, true));
      updates.pop_front();
      REQUIRE(updates.front() == std::pair(parent, true));
      updates.pop_front();
    });
  }
}
