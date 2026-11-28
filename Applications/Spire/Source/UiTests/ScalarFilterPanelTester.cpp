#include <vector>
#include <doctest/doctest.h>
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Tests;

namespace {
  struct Fixture {
    std::shared_ptr<LocalOptionalIntegerModel> m_minimum =
      std::make_shared<LocalOptionalIntegerModel>(10);
    std::shared_ptr<LocalOptionalIntegerModel> m_maximum =
      std::make_shared<LocalOptionalIntegerModel>(20);
    IntegerFilterPanel m_panel = IntegerFilterPanel(m_minimum, m_maximum);
    std::vector<IntegerBox*> m_boxes;
    int m_resets = 0;

    Fixture() {
      for(auto child : m_panel.findChildren<QWidget*>()) {
        if(auto box = dynamic_cast<IntegerBox*>(child)) {
          m_boxes.push_back(box);
        }
      }
      m_panel.connect_reset_signal(std::bind_front(&Fixture::on_reset, this));
    }

    Button& get_reset_button() {
      for(auto child : m_panel.findChildren<QWidget*>()) {
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

    void on_reset() {
      ++m_resets;
    }
  };

  QWidget& get_input(QWidget& widget) {
    auto input = &widget;
    while(auto proxy = input->focusProxy()) {
      input = proxy;
    }
    return *input;
  }
}

TEST_SUITE("ScalarFilterPanel") {
  UI_TEST_CASE("bounds") {
    auto fixture = Fixture();
    REQUIRE(fixture.m_boxes.size() == 2);
    REQUIRE(fixture.m_panel.get_minimum() == fixture.m_minimum);
    REQUIRE(fixture.m_panel.get_maximum() == fixture.m_maximum);
    REQUIRE(fixture.m_boxes[0]->get_text()->get() == "10");
    REQUIRE(fixture.m_boxes[1]->get_text()->get() == "20");
    fixture.m_minimum->set(-5);
    REQUIRE(fixture.m_boxes[0]->get_text()->get() == "-5");
    REQUIRE(fixture.m_boxes[1]->get_text()->get() == "20");
    fixture.m_maximum->set(30);
    REQUIRE(fixture.m_boxes[0]->get_text()->get() == "-5");
    REQUIRE(fixture.m_boxes[1]->get_text()->get() == "30");
    fixture.m_minimum->set(none);
    fixture.m_maximum->set(none);
    REQUIRE(fixture.m_boxes[0]->get_text()->get().isEmpty());
    REQUIRE(fixture.m_boxes[1]->get_text()->get().isEmpty());
    REQUIRE(fixture.m_resets == 0);
  }

  UI_TEST_CASE("editing") {
    auto fixture = Fixture();
    REQUIRE(fixture.m_boxes.size() == 2);
    fixture.m_panel.show();
    auto& minimum = get_input(*fixture.m_boxes[0]);
    auto& maximum = get_input(*fixture.m_boxes[1]);
    press_key(minimum, Qt::Key_A, Qt::ControlModifier);
    type_text(minimum, "12");
    REQUIRE(fixture.m_minimum->get().value_or(-1) == 12);
    REQUIRE(fixture.m_maximum->get().value_or(-1) == 20);
    press_key(maximum, Qt::Key_A, Qt::ControlModifier);
    type_text(maximum, "25");
    REQUIRE(fixture.m_minimum->get().value_or(-1) == 12);
    REQUIRE(fixture.m_maximum->get().value_or(-1) == 25);
    press_key(minimum, Qt::Key_A, Qt::ControlModifier);
    press_key(minimum, Qt::Key_Backspace);
    REQUIRE(!fixture.m_minimum->get());
    REQUIRE(fixture.m_maximum->get().value_or(-1) == 25);
    press_key(maximum, Qt::Key_A, Qt::ControlModifier);
    press_key(maximum, Qt::Key_Backspace);
    REQUIRE(!fixture.m_maximum->get());
    REQUIRE(fixture.m_resets == 0);
  }

  UI_TEST_CASE("reset_request") {
    auto fixture = Fixture();
    press_key(fixture.get_reset_button(), Qt::Key_Return);
    REQUIRE(fixture.m_resets == 1);
    REQUIRE(fixture.m_minimum->get().value_or(-1) == 10);
    REQUIRE(fixture.m_maximum->get().value_or(-1) == 20);
  }
}
