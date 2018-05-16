#include "spire/ui/scroll_bar.hpp"
#include <QEvent>
#include <QMouseEvent>
#include "spire/spire/dimensions.hpp"

using namespace spire;

scroll_bar::scroll_bar(Qt::Orientation orientation, QWidget* parent)
    : QScrollBar(orientation, parent) {
  auto left_margin = 0;
  auto top_margin = 0;
  if(orientation == Qt::Vertical) {
    setFixedWidth(scale_width(12));
    left_margin = scale_width(2);
  } else {
    setFixedHeight(scale_height(12));
    top_margin = scale_height(2);
  }
  setStyleSheet(QString(R"(
    QScrollBar::handle:horizontal, QScrollBar::handle:vertical {
      background-color: #C8C8C8;
      margin-left: %1px;
      margin-top: %2px;
    }

    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
      background: none;
      border: none;
    })").arg(scale_width(left_margin)).arg(scale_height(top_margin)));
  raise();
  setVisible(false);
  parent->setMouseTracking(true);
  parent->installEventFilter(this);
}

bool scroll_bar::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parent()) {
    if(event->type() == QEvent::MouseMove) {
      auto e = static_cast<QMouseEvent*>(event);
      if(geometry().contains(e->pos())) {
        setVisible(true);
      } else {
        setVisible(false);
      }
    } else if(event->type() == QEvent::Wheel) {
      event->accept();
      return true;
    } else if(event->type() == QEvent::Move) {
      if(orientation() == Qt::Vertical) {
        move(static_cast<QWidget*>(parent())->width() - width(), 0);
      } else {
        move(0, static_cast<QWidget*>(parent())->height() - height());
      }
    } else if(event->type() == QEvent::Resize) {
      if(orientation() == Qt::Vertical) {
        resize(width(), static_cast<QWidget*>(parent())->height());
      } else {
        resize(static_cast<QWidget*>(parent())->width(), height());
      }
    }
  }
  return QScrollBar::eventFilter(watched, event);
}

void scroll_bar::leaveEvent(QEvent* event) {
  setVisible(false);
}
