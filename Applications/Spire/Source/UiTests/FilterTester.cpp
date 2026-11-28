#include <limits>
#include <vector>
#include <doctest/doctest.h>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Filter.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Tests;

namespace {
  struct Fixture {
    Filter<int> m_filter;
    std::vector<Filter<int>::Range> m_updates;

    Fixture() {
      m_filter.connect_filter_signal(
        std::bind_front(&Fixture::on_filter, this));
    }

    void on_filter() {
      m_updates.push_back(m_filter.get_current()->get());
    }
  };

  Button& get_reset_button(QWidget& widget) {
    for(auto child : widget.findChildren<QWidget*>()) {
      if(auto panel = dynamic_cast<FilterPanel*>(child)) {
        for(auto child : panel->findChildren<QWidget*>()) {
          auto button = dynamic_cast<Button*>(child);
          if(button && !panel->get_body().isAncestorOf(button)) {
            return *button;
          }
        }
      }
    }
    throw std::runtime_error("Reset button not found.");
  }
}

TEST_SUITE("Filter") {
  TEST_CASE("integer_range") {
    auto filter = Filter<int>();
    REQUIRE(!filter.is_active());
    REQUIRE(!filter.is_filtered(std::numeric_limits<int>::min()));
    REQUIRE(!filter.is_filtered(0));
    REQUIRE(!filter.is_filtered(std::numeric_limits<int>::max()));
    SUBCASE("minimum") {
      filter.get_current()->set(Filter<int>::Range(-5, none));
      REQUIRE(filter.is_filtered(-6));
      REQUIRE(!filter.is_filtered(-5));
      REQUIRE(!filter.is_filtered(std::numeric_limits<int>::max()));
    }
    SUBCASE("maximum") {
      filter.get_current()->set(Filter<int>::Range(none, 5));
      REQUIRE(filter.is_filtered(6));
      REQUIRE(!filter.is_filtered(5));
      REQUIRE(!filter.is_filtered(std::numeric_limits<int>::min()));
    }
    SUBCASE("bounded") {
      filter.get_current()->set(Filter<int>::Range(-5, 5));
      REQUIRE(filter.is_filtered(-6));
      REQUIRE(!filter.is_filtered(-5));
      REQUIRE(!filter.is_filtered(0));
      REQUIRE(!filter.is_filtered(5));
      REQUIRE(filter.is_filtered(6));
    }
    SUBCASE("exact") {
      filter.get_current()->set(Filter<int>::Range(0, 0));
      REQUIRE(filter.is_filtered(-1));
      REQUIRE(!filter.is_filtered(0));
      REQUIRE(filter.is_filtered(1));
    }
    REQUIRE(filter.is_active());
    filter.reset();
    REQUIRE(!filter.is_active());
    REQUIRE(!filter.is_filtered(std::numeric_limits<int>::min()));
    REQUIRE(!filter.is_filtered(std::numeric_limits<int>::max()));
  }

  TEST_CASE("integer_notifications") {
    auto fixture = Fixture();
    auto current = fixture.m_filter.get_current();
    current->set(Filter<int>::Range(10, 20));
    REQUIRE(fixture.m_updates.size() == 1);
    REQUIRE(fixture.m_updates.back() == Filter<int>::Range(10, 20));
    current->set(Filter<int>::Range(15, 25));
    REQUIRE(fixture.m_updates.size() == 2);
    REQUIRE(fixture.m_updates.back() == Filter<int>::Range(15, 25));
    REQUIRE(fixture.m_filter.is_active());
    fixture.m_filter.reset();
    REQUIRE(fixture.m_updates.size() == 3);
    REQUIRE(fixture.m_updates.back() == Filter<int>::Range());
    REQUIRE(!fixture.m_filter.is_active());
  }

  TEST_CASE("integer_current") {
    auto current = std::make_shared<LocalValueModel<Filter<int>::Range>>(
      Filter<int>::Range(10, 20));
    auto filter = Filter<int>(current);
    REQUIRE(filter.is_active());
    REQUIRE(filter.is_filtered(5));
    REQUIRE(!filter.is_filtered(15));
    current->set(Filter<int>::Range(0, 5));
    REQUIRE(!filter.is_filtered(5));
    REQUIRE(filter.is_filtered(15));
    filter.reset();
    REQUIRE(current->get() == Filter<int>::Range());
  }

  UI_TEST_CASE("integer_widget") {
    auto fixture = Fixture();
    auto current = fixture.m_filter.get_current();
    current->set(Filter<int>::Range(10, 20));
    auto parent = QWidget();
    auto panel =
      dynamic_cast<IntegerFilterPanel*>(fixture.m_filter.make_widget(parent));
    REQUIRE(panel);
    REQUIRE(panel->parentWidget() == &parent);
    auto boxes = std::vector<IntegerBox*>();
    for(auto child : panel->findChildren<QWidget*>()) {
      if(auto box = dynamic_cast<IntegerBox*>(child)) {
        boxes.push_back(box);
      }
    }
    REQUIRE(boxes.size() == 2);
    REQUIRE(boxes[0]->get_current()->get().value_or(-1) == 10);
    REQUIRE(boxes[1]->get_current()->get().value_or(-1) == 20);
    fixture.m_updates.clear();
    boxes[0]->get_current()->set(12);
    REQUIRE(current->get() == Filter<int>::Range(12, 20));
    REQUIRE(fixture.m_updates.size() == 1);
    REQUIRE(fixture.m_filter.is_filtered(11));
    current->set(Filter<int>::Range(15, 25));
    REQUIRE(boxes[0]->get_current()->get().value_or(-1) == 15);
    REQUIRE(boxes[1]->get_current()->get().value_or(-1) == 25);
    fixture.m_updates.clear();
    press_key(get_reset_button(*panel), Qt::Key_Return);
    REQUIRE(!fixture.m_filter.is_active());
    REQUIRE(current->get() == Filter<int>::Range());
    REQUIRE(!boxes[0]->get_current()->get());
    REQUIRE(!boxes[1]->get_current()->get());
    REQUIRE(fixture.m_updates.size() == 1);
  }
}
