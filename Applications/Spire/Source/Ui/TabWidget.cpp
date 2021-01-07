#include "Spire/Ui/TabWidget.hpp"
#include <QPainter>
#include <QTabBar>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

TabWidget::TabWidget(QWidget* parent)
  : TabWidget(PaddingStyle::HORIZONTAL, parent) {}

TabWidget::TabWidget(PaddingStyle padding_style, QWidget* parent)
    : QTabWidget(parent) {
  tabBar()->setFixedHeight(scale_height(40));
  auto content_padding = [&] {
    if(padding_style == PaddingStyle::HORIZONTAL) {
      return PADDING();
    }
    return 0;
  }();
  setStyleSheet(QString(R"(
    QTabWidget::pane {
      border: none;
      padding: 0px %7px 0px %7px;
    }

    QTabBar {
      outline: none;
    }

    QTabBar::tab {
      background-color: #EBEBEB;
      font-family: Roboto;
      font-size: %1px;
      height: %2px;
      margin: %3px %4px %3px 0px;
      width: %5px;
    }

    QTabBar::tab:selected {
      background-color: #4B23A0;
      color: white;
    }

    QTabBar::tab:first {
      margin-left: %8px;
    }

    QTabBar::tab:focus {
      background-color: #EBEBEB;
      border: %6px solid #4B23A0;
      color: black;
      padding: -%6px 0px 0px -%6px;
    }

    QTabBar::tab:hover {
      background-color: #4B23A0;
      color: white;
    })").arg(scale_height(12)).arg(scale_height(20)).arg(scale_height(10))
        .arg(scale_width(2)).arg(scale_width(80)).arg(scale_width(1))
        .arg(content_padding).arg(PADDING()));
}

void TabWidget::paintEvent(QPaintEvent* event) {
  QTabWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.setPen(QColor("#E0E0E0"));
  painter.drawLine(tabBar()->width(), tabBar()->height() - scale_height(11),
    width(), tabBar()->height() - scale_height(11));
}
