#include <memory>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/TagBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

namespace {
  TextAreaBox* find_visible_tip() {
    for(auto widget : QApplication::topLevelWidgets()) {
      auto tip = dynamic_cast<InfoTip*>(widget);
      if(!tip || !tip->isVisible() ||
          tip->testAttribute(Qt::WA_DontShowOnScreen)) {
        continue;
      }
      for(auto child : tip->findChildren<QWidget*>()) {
        if(auto body = dynamic_cast<TextAreaBox*>(child)) {
          return body;
        }
      }
    }
    return nullptr;
  }

  QString join(const ArrayListModel<QString>& tags) {
    auto separator = QString(" ") + QChar(0x2022) + " ";
    auto text = QString();
    for(auto i = 0; i != tags.get_size(); ++i) {
      if(i != 0) {
        text += separator;
      }
      text += tags.get(i);
    }
    return text;
  }

  void hover(InfoTipTestDriver& driver, QWidget& window, QWidget& box) {
    move_cursor(window, QPoint(390, 190));
    driver.advance();
    move_cursor(box, QPoint(5, 5));
    driver.advance();
  }
}

TEST_SUITE("TagBox") {
  UI_TEST_CASE("tip") {
    auto driver = InfoTipTestDriver();
    auto window = QWidget();
    window.setGeometry(100, 100, 400, 200);
    auto tags = std::make_shared<ArrayListModel<QString>>();
    tags->push("Apple");
    tags->push("Beige");
    tags->push("Brown");
    tags->push("Cherry");
    auto box = new TagBox(tags, std::make_shared<LocalTextModel>(), &window);
    box->move(10, 10);
    box->setFixedSize(80, box->minimumSizeHint().height());
    window.show();
    QApplication::processEvents();
    hover(driver, window, *box);
    auto tip = find_visible_tip();
    REQUIRE(tip);
    REQUIRE(tip->get_current()->get() == join(*tags));
    move_cursor(window, QPoint(390, 190));
    driver.advance();
    REQUIRE(!find_visible_tip());
    tags->push("Coral");
    hover(driver, window, *box);
    tip = find_visible_tip();
    REQUIRE(tip);
    REQUIRE(tip->get_current()->get() == join(*tags));
    move_cursor(window, QPoint(390, 190));
    driver.advance();
    box->setEnabled(false);
    hover(driver, window, *box);
    REQUIRE(!find_visible_tip());
  }

  UI_TEST_CASE("tip_on_overflow") {
    auto driver = InfoTipTestDriver();
    auto window = QWidget();
    window.setGeometry(100, 100, 400, 200);
    auto tags = std::make_shared<ArrayListModel<QString>>();
    tags->push("A");
    auto box = new TagBox(tags, std::make_shared<LocalTextModel>(), &window);
    box->setGeometry(10, 10, 80, 30);
    window.show();
    QApplication::processEvents();
    hover(driver, window, *box);
    REQUIRE(!find_visible_tip());
    move_cursor(window, QPoint(390, 190));
    driver.advance();
    tags->push("A tag wider than the entire box");
    QApplication::processEvents();
    hover(driver, window, *box);
    auto tip = find_visible_tip();
    REQUIRE(tip);
    REQUIRE(tip->get_current()->get() == join(*tags));
  }
}
