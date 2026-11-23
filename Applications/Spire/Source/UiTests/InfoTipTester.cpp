#include <doctest/doctest.h>
#include <QHelpEvent>
#include <QPointer>
#include <QToolTip>
#include <QWidget>
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

namespace {
  struct Scene {
    QWidget m_window;
    QWidget* m_parent;
    QWidget* m_body;

    Scene()
        : m_parent(new QWidget(&m_window)),
          m_body(new QWidget()) {
      m_window.setGeometry(100, 100, 400, 300);
      m_parent->setGeometry(10, 10, 100, 30);
      m_body->setFixedSize(50, 20);
      m_window.show();
      QApplication::processEvents();
      leave();
    }

    void hover() {
      move_cursor(*m_parent, QPoint(5, 5));
    }

    void leave() {
      move_cursor(m_window, QPoint(390, 290));
    }
  };
}

TEST_SUITE("InfoTip") {
  UI_TEST_CASE("hover") {
    auto driver = InfoTipTestDriver();
    auto scene = Scene();
    auto tip = new InfoTip(scene.m_body, scene.m_parent);
    scene.hover();
    REQUIRE(!tip->isVisible());
    driver.advance();
    REQUIRE(tip->isVisible());
    scene.leave();
    driver.advance();
    REQUIRE(!tip->isVisible());
    scene.hover();
    driver.advance();
    REQUIRE(tip->isVisible());
  }

  UI_TEST_CASE("disabled") {
    auto driver = InfoTipTestDriver();
    auto scene = Scene();
    auto tip = new InfoTip(scene.m_body, scene.m_parent);
    tip->setEnabled(false);
    scene.hover();
    driver.advance();
    REQUIRE(!tip->isVisible());
    tip->setEnabled(true);
    driver.advance();
    REQUIRE(tip->isVisible());
  }

  UI_TEST_CASE("tool_tip") {
    auto scene = Scene();
    scene.m_parent->setToolTip("Tool tip");
    auto tip = new InfoTip(scene.m_body, scene.m_parent);
    auto event = QHelpEvent(QEvent::ToolTip, QPoint(5, 5),
      scene.m_parent->mapToGlobal(QPoint(5, 5)));
    QApplication::sendEvent(scene.m_parent, &event);
    REQUIRE(!QToolTip::isVisible());
  }

  UI_TEST_CASE("hovered_on_construction") {
    auto driver = InfoTipTestDriver();
    auto scene = Scene();
    scene.hover();
    auto tip = new InfoTip(scene.m_body, scene.m_parent);
    driver.advance();
    REQUIRE(tip->isVisible());
  }

  UI_TEST_CASE("interactive") {
    auto driver = InfoTipTestDriver();
    auto scene = Scene();
    auto tip = new InfoTip(scene.m_body, scene.m_parent);
    scene.hover();
    driver.advance();
    REQUIRE(tip->isVisible());
    move_cursor(*tip, tip->rect().center());
    driver.advance();
    REQUIRE(!tip->isVisible());
    tip->set_interactive(true);
    scene.hover();
    driver.advance();
    REQUIRE(tip->isVisible());
    move_cursor(*tip, tip->rect().center());
    driver.advance();
    REQUIRE(tip->isVisible());
    scene.leave();
    driver.advance();
    REQUIRE(!tip->isVisible());
  }

  UI_TEST_CASE("deletion") {
    auto scene = Scene();
    auto body_tip = QPointer(new InfoTip(scene.m_body, scene.m_parent));
    delete scene.m_body;
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(!body_tip);
    auto parent_tip = QPointer(new InfoTip(new QWidget(), scene.m_parent));
    delete scene.m_parent;
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(!parent_tip);
  }
}
