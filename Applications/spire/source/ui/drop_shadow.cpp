#include "spire/ui/drop_shadow.hpp"
#include <QEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QRadialGradient>
#include "spire/spire/dimensions.hpp"

using namespace spire;

namespace {
  const auto TOP_STOPS = QGradientStops({
    QGradientStop(0.0, QColor(0, 0, 0, 43)),
    QGradientStop(0.05, QColor(0, 0, 0, 15)),
    QGradientStop(0.1, QColor(0, 0, 0, 8)),
    QGradientStop(0.20, QColor(0, 0, 0, 4)),
    QGradientStop(0.50, QColor(0, 0, 0, 1)),
    QGradientStop(1, Qt::transparent)});

  const auto BOTTOM_STOPS = QGradientStops({
    QGradientStop(0.0, QColor(0, 0, 0, 43)),
    QGradientStop(0.10, QColor(0, 0, 0, 15)),
    QGradientStop(0.16, QColor(0, 0, 0, 8)),
    QGradientStop(0.24, QColor(0, 0, 0, 4)),
    QGradientStop(0.50, QColor(0, 0, 0, 1)),
    QGradientStop(1, Qt::transparent)});
}

drop_shadow::drop_shadow(QWidget* parent)
    : drop_shadow(false, true, parent) {}

drop_shadow::drop_shadow(bool has_top, QWidget* parent)
    : drop_shadow(false, has_top, parent) {}

drop_shadow::drop_shadow(bool is_menu_shadow, bool has_top, QWidget* parent)
    : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint | Qt::Tool),
      m_parent(parent),
      m_has_top(has_top),
      m_is_menu_shadow(is_menu_shadow),
      m_is_visible(false) {
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);
  m_parent->window()->installEventFilter(this);
}

bool drop_shadow::event(QEvent* event) {
  if(event->type() == QEvent::WindowActivate) {
    m_parent->activateWindow();
  }
  return QWidget::event(event);
}

bool drop_shadow::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Move) {
    follow_parent();
  } else if(event->type() == QEvent::Resize) {
    auto parent_size = m_parent->window()->frameGeometry().size();
    resize(parent_size.width() + 2 * shadow_size().width(),
      parent_size.height() + 2 * shadow_size().height());
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
  auto parent_size = m_parent->frameGeometry().size();
  auto shadow_size = this->shadow_size();
  if(m_has_top) {
    QRect top_rect(QPoint(shadow_size.width(), 0),
      QSize(parent_size.width(), shadow_size.height()));
    QLinearGradient top_gradient(top_rect.left(), top_rect.bottom(),
      top_rect.left(), top_rect.top());
    top_gradient.setStops(TOP_STOPS);
    painter.fillRect(top_rect, top_gradient);
    QRadialGradient top_left_corner(shadow_size.width(),
      shadow_size.height(), shadow_size.width(), shadow_size.width(),
      shadow_size.height());
    top_left_corner.setStops(TOP_STOPS);
    painter.fillRect(0, 0, shadow_size.width(), shadow_size.height(),
      top_left_corner);
    QRadialGradient top_right_corner(
      parent_size.width() + shadow_size.width(),
      shadow_size.height(), shadow_size.width(),
    parent_size.width() + shadow_size.width(), shadow_size.height());
    top_right_corner.setStops(TOP_STOPS);
    painter.fillRect(parent_size.width() + shadow_size.width(), 0,
      shadow_size.width(), shadow_size.height(), top_right_corner);
  }
  QRect right_rect(QPoint(
    shadow_size.width() + parent_size.width(), shadow_size.height()),
    QSize(shadow_size.width(), parent_size.height()));
  QLinearGradient right_gradient(right_rect.x(), right_rect.y(),
    right_rect.x() + right_rect.width(), right_rect.y());
  right_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(right_rect, right_gradient);
  QRect bottom_rect(QPoint(
    shadow_size.width(), shadow_size.height() + parent_size.height()),
    QSize(parent_size.width(), shadow_size.height()));
  QLinearGradient bottom_gradient(bottom_rect.x(), bottom_rect.y(),
    bottom_rect.x(), bottom_rect.bottom());
  bottom_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(bottom_rect, bottom_gradient);
  QRect left_rect(QPoint(0, shadow_size.height()),
    QSize(shadow_size.width(), parent_size.height()));
  QLinearGradient left_gradient(left_rect.right(), left_rect.y(),
    left_rect.x(), left_rect.y());
  left_gradient.setStops(TOP_STOPS);
  painter.fillRect(left_rect, left_gradient);
  QRadialGradient bottom_right_corner(
    parent_size.width() + shadow_size.width(),
    parent_size.height() + shadow_size.height(), shadow_size.width(),
    parent_size.width() + shadow_size.width(),
    parent_size.height() + shadow_size.height());
  bottom_right_corner.setStops(TOP_STOPS);
  painter.fillRect(parent_size.width() + shadow_size.width(),
    parent_size.height() + shadow_size.height(), shadow_size.width(),
    shadow_size.height(), bottom_right_corner);
  QRadialGradient bottom_left_corner(shadow_size.width(),
    parent_size.height() + shadow_size.height(), shadow_size.width(),
    shadow_size.width(), parent_size.height() + shadow_size.height());
  bottom_left_corner.setStops(TOP_STOPS);
  painter.fillRect(0, parent_size.height() + shadow_size.height(),
    shadow_size.width(), shadow_size.height(), bottom_left_corner);
  QWidget::paintEvent(event);
}

void drop_shadow::follow_parent() {
  auto top_left = m_parent->window()->frameGeometry().topLeft();
  move(top_left.x() - shadow_size().width(),
    top_left.y() - shadow_size().height());
}

QSize drop_shadow::shadow_size() {
  if(m_is_menu_shadow) {
    return scale(7, 7);
  } else {
    return scale(14, 14);
  }
}
