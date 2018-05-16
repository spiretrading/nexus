#include "spire/ui/scroll_bar.hpp"
#include <QEvent>
#include <QMouseEvent>
#include "spire/spire/dimensions.hpp"

using namespace spire;

scroll_bar::scroll_bar(Qt::Orientation orientation, QWidget* parent)
    : QScrollBar(orientation, parent) {
  setStyleSheet(QString(R"(
    QScrollBar::handle:vertical {
      background-color: #C8C8C8;
      margin-left: %1px;
    }

    QScrollBar::add-line:vertical {
      background: none;
      border: none;
    }

    QScrollBar::sub-line:vertical {
      background: none;
      border: none;
    })").arg(scale_width(2)));
  setFixedWidth(scale_width(12));
  resize(width(), parent->height());
  move(parent->width(), 0);
  raise();
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
    } else if(event->type() == QEvent::Move) {
      move(static_cast<QWidget*>(parent())->width() - width(), 0);
    } else if(event->type() == QEvent::Resize) {
      resize(width(), static_cast<QWidget*>(parent())->height());
    }
  }
  return QScrollBar::eventFilter(watched, event);
}
