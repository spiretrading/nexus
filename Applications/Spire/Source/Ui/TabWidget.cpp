#include "Spire/Ui/TabWidget.hpp"
#include <QKeyEvent>
#include <QTabBar>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

TabWidget::TabWidget(QWidget* parent)
    : QTabWidget(parent),
      m_last_focus_was_key(false) {
  tabBar()->setFixedHeight(scale_height(40));
  setStyleSheet(QString(R"(
    QWidget {
      outline: none;
    }

    QTabWidget::pane {
      border: none;
    }

    QTabBar::tab {
      background-color: #EBEBEB;
      font-family: Roboto;
      font-size: %1px;
      height: %2px;
      margin: %3px %4px %3px 0px;
      width: %5px;
    }

    QTabBar::tab:focus {
      border: %6px solid #4B23A0;
      padding: -%6px 0px 0px -%6px;
    }

    QTabBar::tab:hover {
      background-color: #DBDBDB;
      color: black;
    }

    QTabBar::tab:selected {
      background-color: #F5F5F5;
      color: #4B23A0;
    })").arg(scale_height(12)).arg(scale_height(20)).arg(scale_height(10))
        .arg(scale_width(2)).arg(scale_width(80)).arg(scale_width(1)));
  connect(this, &QTabWidget::tabBarClicked, this,
    &TabWidget::on_tab_bar_clicked);
  connect(this, &QTabWidget::currentChanged, this,
    &TabWidget::on_tab_changed);
}

void TabWidget::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
    m_last_focus_was_key = true;
  }
}

void TabWidget::mousePressEvent(QMouseEvent* event) {
  m_last_focus_was_key = false;
}

void TabWidget::on_tab_bar_clicked(int index) {
  if(widget(index) != nullptr) {
    widget(index)->setFocus();
  }
}

void TabWidget::on_tab_changed() {
  if(m_last_focus_was_key) {
    tabBar()->setFocus();
  }
}
