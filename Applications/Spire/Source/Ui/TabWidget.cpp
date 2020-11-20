#include "Spire/Ui/TabWidget.hpp"
#include <QPainter>
#include <QTabBar>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

TabWidget::TabWidget(QWidget* parent)
    : QTabWidget(parent) {
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

    QTabBar::tab:hover {
      background-color: #DBDBDB;
      color: black;
    }

    QTabBar::tab:selected {
      background-color: #8C8C8C;
      color: white;
    }

    QTabBar::tab:focus {
      background-color: #4B23A0;
      color: white;
    })").arg(scale_height(12)).arg(scale_height(20)).arg(scale_height(10))
        .arg(scale_width(2)).arg(scale_width(80)));
}

void TabWidget::paintEvent(QPaintEvent* event) {
  QTabWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.setPen(QColor("#E0E0E0"));
  painter.drawLine(tabBar()->width(), tabBar()->height() - scale_height(11),
    width(), tabBar()->height() - scale_height(11));
}
