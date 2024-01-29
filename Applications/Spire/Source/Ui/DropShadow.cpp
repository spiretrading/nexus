#include "Spire/Ui/DropShadow.hpp"
#include <QApplication>
#include <QEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QRadialGradient>
#include <qt_windows.h>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

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

DropShadow::DropShadow(QWidget* parent)
    : DropShadow(false, true, parent) {}

DropShadow::DropShadow(bool has_top, QWidget* parent)
    : DropShadow(false, has_top, parent) {}

DropShadow::DropShadow(bool is_menu_shadow, bool has_top, QWidget* parent)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::Tool |
        Qt::WindowStaysOnTopHint),
      m_parent(parent),
      m_has_top(has_top),
      m_is_menu_shadow(is_menu_shadow),
      m_is_visible(false) {
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);
  m_parent->window()->installEventFilter(this);
  connect(m_parent, qOverload<QObject*>(&QObject::destroyed),
    [=] (auto object) { deleteLater(); });
  qApp->installNativeEventFilter(this);
}

bool DropShadow::event(QEvent* event) {
  if(event->type() == QEvent::WindowActivate) {
    m_parent->activateWindow();
  }
  return QWidget::event(event);
}

bool DropShadow::eventFilter(QObject* watched, QEvent* event) {
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

void DropShadow::hideEvent(QHideEvent* event) {
  m_is_visible = false;
}

bool DropShadow::nativeEventFilter(const QByteArray& event_type, void* message,
    long* result) {
  auto msg = static_cast<MSG*>(message);
  auto handle = reinterpret_cast<HWND>(window()->effectiveWinId());
  if(handle == msg->hwnd && msg->message == WM_WINDOWPOSCHANGING) {
    auto pos = reinterpret_cast<WINDOWPOS*>(msg->lParam);
    pos->hwndInsertAfter = reinterpret_cast<HWND>(m_parent->effectiveWinId());
  }
  return false;
}

void DropShadow::paintEvent(QPaintEvent* event) {
  if(!m_is_visible) {
    follow_parent();
    m_is_visible = true;
  }
  QPainter painter(this);
  auto parent_size = m_parent->size();
  auto shadow_size = this->shadow_size();
  auto right_start = shadow_size.width() + parent_size.width();
  auto bottom_start = shadow_size.height() + parent_size.height();
  QRect top_left_rect(QPoint(0, 0), shadow_size);
  QRect top_rect(QPoint(shadow_size.width(), 0),
    QSize(parent_size.width(), shadow_size.height()));
  if(m_has_top) {
    QRadialGradient top_left_gradient(top_left_rect.bottomRight(),
      top_left_rect.width());
    top_left_gradient.setStops(TOP_STOPS);
    painter.fillRect(top_left_rect, top_left_gradient);
    QLinearGradient top_gradient(top_rect.bottomLeft(), top_rect.topLeft());
    top_gradient.setStops(TOP_STOPS);
    painter.fillRect(top_rect, top_gradient);
    auto top_right_rect = top_left_rect;
    top_right_rect.translate(right_start, 0);
    QRadialGradient top_right_gradient(top_right_rect.bottomLeft(),
      top_right_rect.width());
    top_right_gradient.setStops(TOP_STOPS);
    painter.fillRect(top_right_rect, top_right_gradient);
  }
  auto bottom_left_rect = top_left_rect;
  bottom_left_rect.translate(0, bottom_start);
  QRadialGradient bottom_left_gradient(bottom_left_rect.topRight(),
    bottom_left_rect.width());
  bottom_left_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(bottom_left_rect, bottom_left_gradient);
  auto bottom_rect = top_rect;
  bottom_rect.translate(0, bottom_start);
  QLinearGradient bottom_gradient(bottom_rect.topLeft(),
    bottom_rect.bottomLeft());
  bottom_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(bottom_rect, bottom_gradient);
  auto bottom_right_rect = top_left_rect;
  bottom_right_rect.translate(right_start, bottom_start);
  QRadialGradient bottom_right_gradient(bottom_right_rect.topLeft(),
    bottom_right_rect.width());
  bottom_right_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(bottom_right_rect, bottom_right_gradient);
  QRect left_rect(QPoint(0, shadow_size.height()),
    QSize(shadow_size.width(), parent_size.height()));
  QLinearGradient left_gradient(left_rect.topRight(), left_rect.topLeft());
  left_gradient.setStops(TOP_STOPS);
  painter.fillRect(left_rect, left_gradient);
  auto right_rect = left_rect;
  right_rect.translate(right_start, 0);
  QLinearGradient right_gradient(right_rect.topLeft(), right_rect.topRight());
  right_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(right_rect, right_gradient);
  QWidget::paintEvent(event);
}

void DropShadow::follow_parent() {
  raise();
  auto top_left = m_parent->window()->frameGeometry().topLeft();
  move(top_left.x() - shadow_size().width(),
    top_left.y() - shadow_size().height());
}

QSize DropShadow::shadow_size() {
  if(m_is_menu_shadow) {
    return scale(7, 7);
  } else {
    return scale(14, 14);
  }
}
