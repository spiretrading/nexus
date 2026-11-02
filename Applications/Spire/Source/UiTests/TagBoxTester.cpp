#include <memory>
#include <doctest/doctest.h>
#include <QElapsedTimer>
#include <QWidget>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/TagBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

namespace {
  void wait(int milliseconds) {
    auto timer = QElapsedTimer();
    timer.start();
    while(timer.elapsed() < milliseconds) {
      QApplication::processEvents(QEventLoop::AllEvents, 10);
    }
  }

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

  void hover(QWidget& window, QWidget& box) {
    move_cursor(window, QPoint(390, 190));
    wait(400);
    move_cursor(box, QPoint(5, 5));
    wait(800);
  }
}

TEST_SUITE("TagBox") {
  UI_TEST_CASE("tip") {
    auto window = QWidget();
    window.setGeometry(100, 100, 400, 200);
    auto tags = std::make_shared<ArrayListModel<QString>>();
    tags->push("Apple");
    tags->push("Beige");
    tags->push("Brown");
    tags->push("Cherry");
    auto box = new TagBox(tags, std::make_shared<LocalTextModel>(), &window);
    box->setGeometry(10, 10, 80, 30);
    window.show();
    QApplication::processEvents();
    hover(window, *box);
    auto tip = find_visible_tip();
    REQUIRE(tip);
    REQUIRE(tip->get_current()->get() == join(*tags));
    move_cursor(window, QPoint(390, 190));
    wait(400);
    REQUIRE(!find_visible_tip());
    tags->push("Coral");
    hover(window, *box);
    tip = find_visible_tip();
    REQUIRE(tip);
    REQUIRE(tip->get_current()->get() == join(*tags));
    move_cursor(window, QPoint(390, 190));
    wait(400);
    box->setEnabled(false);
    hover(window, *box);
    REQUIRE(!find_visible_tip());
  }

  UI_TEST_CASE("tip_on_overflow") {
    auto window = QWidget();
    window.setGeometry(100, 100, 400, 200);
    auto tags = std::make_shared<ArrayListModel<QString>>();
    tags->push("A");
    auto box = new TagBox(tags, std::make_shared<LocalTextModel>(), &window);
    box->setGeometry(10, 10, 80, 30);
    window.show();
    QApplication::processEvents();
    hover(window, *box);
    REQUIRE(!find_visible_tip());
    move_cursor(window, QPoint(390, 190));
    wait(400);
    tags->push("Cherry");
    QApplication::processEvents();
    hover(window, *box);
    auto tip = find_visible_tip();
    REQUIRE(tip);
    REQUIRE(tip->get_current()->get() == join(*tags));
  }
}
