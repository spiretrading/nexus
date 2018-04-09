#include "spire/ui/drop_shadow.hpp"
#include <QEvent>
#include <QLinearGradient>
#include <QPainter>

using namespace spire;

namespace {
  const auto shadow_size = QSize(20, 20);
  const auto gradient_color = QColor(0, 0, 0, 100);
}

drop_shadow::drop_shadow(QWidget* parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint),
      m_is_visible(false) {
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);
  parent->installEventFilter(this);
}

bool drop_shadow::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Move) {
    follow_parent();
  } else if(event->type() == QEvent::Resize) {
    auto parent_size = static_cast<QWidget*>(
      parent())->window()->frameGeometry().size();
    resize(parent_size.width() + 2 * shadow_size.width(),
      parent_size.height() + 2 * shadow_size.height());
    follow_parent();
  } else if(event->type() == QEvent::Show) {
    show();
  } else if(event->type() == QEvent::Hide) {
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

void drop_shadow::hideEvent(QHideEvent* event) {
  m_is_visible = false;
}

void drop_shadow::paintEvent(QPaintEvent* event) {
  if(!m_is_visible) {
    follow_parent();
    m_is_visible = true;
  }
  QPainter painter(this);
  auto parent_size = static_cast<QWidget*>(parent())->size();
  QRect top_rect(QPoint(shadow_size.width(), 0),
      QSize(parent_size.width(), shadow_size.height()));
  QLinearGradient top_gradient(top_rect.left(), top_rect.bottom(),
    top_rect.left(), top_rect.top());
  top_gradient.setColorAt(0, gradient_color);
  top_gradient.setColorAt(1, Qt::transparent);
  painter.fillRect(top_rect, top_gradient);
  QRect right_rect(QPoint(
        shadow_size.width() + parent_size.width(), shadow_size.height()),
      QSize(shadow_size.width(), parent_size.height()));
  QLinearGradient right_gradient(right_rect.x(), right_rect.y(),
    right_rect.x() + right_rect.width(), right_rect.y());
  right_gradient.setColorAt(0, gradient_color);
  right_gradient.setColorAt(1, Qt::transparent);
  painter.fillRect(right_rect, right_gradient);
  QRect bottom_rect(QPoint(
        shadow_size.width(), shadow_size.height() + parent_size.height()),
      QSize(parent_size.width(), shadow_size.height()));
  QLinearGradient bottom_gradient(bottom_rect.x(), bottom_rect.y(),
    bottom_rect.x(), bottom_rect.bottom());
  bottom_gradient.setColorAt(0, gradient_color);
  bottom_gradient.setColorAt(1, Qt::transparent);
  painter.fillRect(bottom_rect, bottom_gradient);
  QRect left_rect(QPoint(0, shadow_size.height()),
      QSize(shadow_size.width(), parent_size.height()));
  QLinearGradient left_gradient(left_rect.right(), left_rect.y(),
    left_rect.x(), left_rect.y());
  left_gradient.setColorAt(0, gradient_color);
  left_gradient.setColorAt(1, Qt::transparent);
  painter.fillRect(left_rect, left_gradient);
  QWidget::paintEvent(event);
}

void drop_shadow::follow_parent() {
  auto parent_widget = static_cast<QWidget*>(parent());
  auto top_left = parent_widget->window()->frameGeometry().topLeft();
  move(top_left.x() - shadow_size.width(),
    top_left.y() - shadow_size.height());
}
