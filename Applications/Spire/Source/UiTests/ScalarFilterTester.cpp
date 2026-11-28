#include <doctest/doctest.h>
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ScalarFilter.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Tests;

namespace {
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

TEST_SUITE("ScalarFilter") {
  TEST_CASE("money_range") {
    using Filter = ScalarFilter<MoneyBox>;
    auto minimum = 10 * Money::CENT;
    auto maximum = 20 * Money::CENT;
    auto filter = Filter(Filter::Range(minimum, maximum));
    REQUIRE(filter.is_active());
    REQUIRE(filter.is_filtered(minimum - Money::CENT));
    REQUIRE(!filter.is_filtered(minimum));
    REQUIRE(!filter.is_filtered(15 * Money::CENT));
    REQUIRE(!filter.is_filtered(maximum));
    REQUIRE(filter.is_filtered(maximum + Money::CENT));
    filter.reset();
    REQUIRE(!filter.is_active());
    REQUIRE(!filter.is_filtered(minimum - Money::CENT));
    REQUIRE(!filter.is_filtered(maximum + Money::CENT));
  }

  UI_TEST_CASE("widget_lifetime") {
    using Filter = ScalarFilter<IntegerBox>;
    auto current = std::make_shared<LocalValueModel<Filter::Range>>(
      Filter::Range(10, 20));
    auto parent = QWidget();
    auto first = static_cast<IntegerFilterPanel*>(nullptr);
    auto second = static_cast<IntegerFilterPanel*>(nullptr);
    {
      auto filter = Filter(current);
      first = dynamic_cast<IntegerFilterPanel*>(filter.make_widget(parent));
      second = dynamic_cast<IntegerFilterPanel*>(filter.make_widget(parent));
    }
    REQUIRE(first);
    REQUIRE(second);
    first->get_minimum()->set(12);
    REQUIRE(current->get() == Filter::Range(12, 20));
    REQUIRE(second->get_minimum()->get().value_or(-1) == 12);
    second->get_maximum()->set(25);
    REQUIRE(current->get() == Filter::Range(12, 25));
    REQUIRE(first->get_maximum()->get().value_or(-1) == 25);
    press_key(get_reset_button(*first), Qt::Key_Return);
    REQUIRE(current->get() == Filter::Range());
    REQUIRE(!first->get_minimum()->get());
    REQUIRE(!first->get_maximum()->get());
    REQUIRE(!second->get_minimum()->get());
    REQUIRE(!second->get_maximum()->get());
  }

}
