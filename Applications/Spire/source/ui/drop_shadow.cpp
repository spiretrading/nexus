#include "spire/ui/drop_shadow.hpp"
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>

using namespace spire;

drop_shadow::drop_shadow(const QMargins& margins, QWidget* parent)
    : QWidget(parent),
      m_margins(margins),
      m_is_shown(false) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);
  m_inner_widget = new QWidget(this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(margins);
  layout->setSpacing(0);
  layout->addWidget(m_inner_widget);
  auto drop_shadow = new QGraphicsDropShadowEffect(this);
  drop_shadow->setBlurRadius(120);
  drop_shadow->setXOffset(0);
  drop_shadow->setYOffset(0);
  drop_shadow->setColor(QColor(255, 0, 0, 200));
  m_inner_widget->setGraphicsEffect(drop_shadow);
  //m_inner_widget->setAttribute(Qt::WA_TranslucentBackground);
  parent->installEventFilter(this);
}

bool drop_shadow::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parent()) {
    if(event->type() == QEvent::Move) {
      move_to_parent();
    }
    if(event->type() == QEvent::Resize) {
      // TODO: resize relative to frame size
      auto parent_size = static_cast<QWidget*>(parent())->size();
      resize(
        parent_size.width() + m_margins.left() + m_margins.right(),
        parent_size.height() + m_margins.top() + m_margins.bottom());
    }
    if(event->type() == QEvent::Paint) {
      if(!m_is_shown) {
        move_to_parent();
        show();
        m_is_shown = true;
      }
    }
    if(event->type() == QEvent::WindowDeactivate) {
      m_is_shown = false;
      hide();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void drop_shadow::move_to_parent() {
  auto parent_widget = static_cast<QWidget*>(parent());
  auto top_left = parent_widget->mapToGlobal(QPoint(0, 0));
  auto x_pos = top_left.x();
  auto y_pos = top_left.y();
  move(x_pos - m_margins.left(), y_pos - m_margins.top());
}
