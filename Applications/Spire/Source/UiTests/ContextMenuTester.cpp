#include <QApplication>
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

TEST_SUITE("ContextMenu") {
  UI_TEST_CASE("action_submission_during_reset") {
    auto button = MenuButton(*new QWidget());
    button.show();
    auto& menu = button.get_menu();
    auto calls = 0;
    auto rebuild = [&] {
      menu.reset();
      menu.add_disabled_action("Replacement");
    };
    auto action = [&, count = 0] () mutable {
      calls = ++count;
      if(count == 2) {
        rebuild();
      }
    };
    SUBCASE("default_view") {
      menu.add_action("Original", action);
    }
    SUBCASE("custom_view") {
      menu.add_action("Original", action, new QWidget());
    }
    auto submissions = std::vector<QString>();
    menu.connect_submit_signal([&] (const auto&, const QString& label) {
      submissions.push_back(label);
    });
    for(auto i = 0; i != 2; ++i) {
      press_key(button, Qt::Key_Space);
      release_key(button, Qt::Key_Space);
      QApplication::processEvents();
      REQUIRE(menu.isVisible());
      auto item = menu.get_menu_item(0);
      REQUIRE(item);
      press_key(*item, Qt::Key_Return);
    }
    REQUIRE(calls == 2);
    REQUIRE(submissions == std::vector<QString>{"Original", "Original"});
    REQUIRE_FALSE(menu.get_menu_item(0)->isEnabled());
  }

  UI_TEST_CASE("check_box_submission_during_reset") {
    auto button = MenuButton(*new QWidget());
    button.show();
    auto& menu = button.get_menu();
    menu.add_check_box("Original")->connect_update_signal([&] (bool checked) {
      REQUIRE(checked);
      menu.reset();
      menu.add_disabled_action("Replacement");
    });
    auto submitted = QString();
    menu.connect_submit_signal([&] (const auto&, const QString& label) {
      submitted = label;
    });
    press_key(button, Qt::Key_Space);
    release_key(button, Qt::Key_Space);
    QApplication::processEvents();
    REQUIRE(menu.isVisible());
    auto item = menu.get_menu_item(0);
    REQUIRE(item);
    press_key(*item, Qt::Key_Return);
    REQUIRE(submitted == "Original");
    REQUIRE_FALSE(menu.get_menu_item(0)->isEnabled());
  }
}
