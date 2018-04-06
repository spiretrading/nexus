#include "spire/ui/drop_shadow.hpp"
#include <QEvent>
#include <QLinearGradient>
#include <QPainter>

using namespace spire;

namespace {
  const auto shadow_size = QSize(20, 20);
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
  // maybe make this a member
  auto parent_size = static_cast<QWidget*>(parent())->size();

  QRect rect(QPoint(shadow_size.width(), 0),
      QSize(parent_size.width(), shadow_size.height()));
  // The change from full red to transparent looks kind of sudden, can either
  // add more stops in the gradient or use a less intense shade of red, e.g.,
  // QColor(125, 0, 0, 100)
  QLinearGradient gradient(rect.left(), rect.bottom(), rect.left(), rect.top());
  gradient.setColorAt(0, QColor(255, 0, 0, 100)); // alpha 100/255, roughly 40%
  gradient.setColorAt(1, Qt::transparent);
  painter.fillRect(rect, gradient);
  painter.fillRect(QRect(QPoint(0, shadow_size.height()),
    QSize(shadow_size.width(), parent_size.height())), Qt::red);
  painter.fillRect(QRect(
    QPoint(shadow_size.width(), shadow_size.height() + parent_size.height()),
    QSize(parent_size.width(), shadow_size.height())), Qt::red);
  painter.fillRect(QRect(
    QPoint(parent_size.width() + shadow_size.width(), shadow_size.height()),
    QSize(shadow_size.width(), parent_size.height())), Qt::red);
  QWidget::paintEvent(event);
}

void drop_shadow::follow_parent() {
  auto parent_widget = static_cast<QWidget*>(parent());
  auto top_left = parent_widget->window()->frameGeometry().topLeft();
  move(top_left.x() - shadow_size.width(),
    top_left.y() - shadow_size.height());
}
