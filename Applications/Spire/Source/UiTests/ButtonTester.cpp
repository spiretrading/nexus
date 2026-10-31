#include <memory>
#include <doctest/doctest.h>
#include <QImage>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Tests;

TEST_SUITE("Button") {
  UI_TEST_CASE("body") {
    auto body = new QWidget();
    auto button = Button(body);
    REQUIRE(&button.get_body() == body);
    REQUIRE(body->parentWidget() == &button);
    REQUIRE(is_match(*body, Body()));
  }

  UI_TEST_CASE("mouse_click") {
    auto button = Button(new QWidget());
    button.resize(100, 30);
    button.show();
    auto clicks = 0;
    button.connect_click_signal([&] {
      ++clicks;
    });
    move_cursor(button, button.rect().center());
    press_mouse(button, Qt::LeftButton);
    REQUIRE(clicks == 0);
    release_mouse(button, Qt::LeftButton);
    REQUIRE(clicks == 1);
    press_mouse(button, Qt::LeftButton);
    move_cursor(button, QPoint(button.width() + 10, button.height() + 10));
    release_mouse(button, Qt::LeftButton);
    REQUIRE(clicks == 1);
    move_cursor(button, button.rect().center());
    press_mouse(button, Qt::RightButton);
    release_mouse(button, Qt::RightButton);
    REQUIRE(clicks == 1);
  }

  UI_TEST_CASE("keyboard_click") {
    auto button = Button(new QWidget());
    auto clicks = 0;
    button.connect_click_signal([&] {
      ++clicks;
    });
    press_key(button, Qt::Key_Space);
    REQUIRE(clicks == 0);
    press_key(button, Qt::Key_Space, Qt::NoModifier, true);
    release_key(button, Qt::Key_Space, Qt::NoModifier, true);
    REQUIRE(clicks == 0);
    release_key(button, Qt::Key_Space);
    REQUIRE(clicks == 1);
    press_key(button, Qt::Key_Return);
    REQUIRE(clicks == 2);
    press_key(button, Qt::Key_Enter);
    REQUIRE(clicks == 3);
    press_key(button, Qt::Key_Return, Qt::ShiftModifier);
    REQUIRE(clicks == 3);
    press_key(button, Qt::Key_Return, Qt::NoModifier, true);
    REQUIRE(clicks == 3);
  }

  UI_TEST_CASE("disabled") {
    auto button = Button(new QWidget());
    button.resize(100, 30);
    button.show();
    button.setEnabled(false);
    auto clicks = 0;
    button.connect_click_signal([&] {
      ++clicks;
    });
    move_cursor(button, button.rect().center());
    press_mouse(button, Qt::LeftButton);
    release_mouse(button, Qt::LeftButton);
    press_key(button, Qt::Key_Space);
    release_key(button, Qt::Key_Space);
    press_key(button, Qt::Key_Return);
    REQUIRE(clicks == 0);
  }

  UI_TEST_CASE("press_state") {
    auto button = Button(new QWidget());
    button.resize(100, 30);
    button.show();
    move_cursor(button, button.rect().center());
    REQUIRE(!is_match(button, Press()));
    press_mouse(button, Qt::LeftButton);
    QApplication::processEvents();
    REQUIRE(is_match(button, Press()));
    release_mouse(button, Qt::LeftButton);
    QApplication::processEvents();
    REQUIRE(!is_match(button, Press()));
    press_key(button, Qt::Key_Space);
    QApplication::processEvents();
    REQUIRE(is_match(button, Press()));
    release_key(button, Qt::Key_Space);
    QApplication::processEvents();
    REQUIRE(!is_match(button, Press()));
  }

  UI_TEST_CASE("make_label_button") {
    auto button = std::unique_ptr<Button>(make_label_button("Submit"));
    auto label = dynamic_cast<TextBox*>(&button->get_body());
    REQUIRE(label);
    REQUIRE(label->get_current()->get() == "Submit");
    REQUIRE(button->minimumHeight() == scale_height(26));
    REQUIRE(button->maximumHeight() == scale_height(26));
  }

  UI_TEST_CASE("make_icon_button") {
    auto button = std::unique_ptr<Button>(
      make_icon_button(QImage(16, 16, QImage::Format_ARGB32)));
    auto box = dynamic_cast<Box*>(&button->get_body());
    REQUIRE(box);
    REQUIRE(dynamic_cast<Icon*>(box->get_body()));
  }
}
