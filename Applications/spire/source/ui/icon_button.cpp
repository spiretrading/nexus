#include "spire/ui/icon_button.hpp"
#include <QColor>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QSize>
#include <QtSvg/QSvgRenderer>

using namespace boost;
using namespace boost::signals2;
using namespace spire;

icon_button::icon_button(const QString& default_icon, const QString& hover_icon,
    int width, int height, QWidget* parent)
    : icon_button(default_icon, hover_icon, width, height,
                  QRect(0, 0, width, height), parent) {}

icon_button::icon_button(const QString& default_icon, const QString& hover_icon,
    int width, int height, const QRect& draw_rect, QWidget* parent)
    : QWidget(parent),
      m_clickable(true),
      m_focusable(false) {
  setFocusPolicy(Qt::ClickFocus);
  auto layout = new QHBoxLayout(this);
  layout->setMargin(0);
  m_label = new QLabel(this);
  auto renderer = new QSvgRenderer(default_icon, this);
  auto default_image = QImage(QSize(width, height), QImage::Format_ARGB32);
  default_image.fill(QColor(0, 0, 0, 0));
  QPainter p1(&default_image);
  renderer->render(&p1, draw_rect);
  m_default_icon = default_image;
  renderer->load(hover_icon);
  auto hover_image = QImage(QSize(width, height), QImage::Format_ARGB32);
  hover_image.fill(QColor(0, 0, 0, 0));
  QPainter p2(&hover_image);
  renderer->render(&p2, draw_rect);
  m_hover_icon = hover_image;
  m_label->setPixmap(QPixmap::fromImage(m_default_icon));
  layout->addWidget(m_label);
}

connection icon_button::connect_clicked_signal(
    const clicked_signal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void icon_button::enterEvent(QEvent* event) {
  m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
}

void icon_button::set_icon(bool is_default) {
  if(is_default) {
    m_label->setPixmap(QPixmap::fromImage(m_default_icon));
  } else {
    m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
  }
  m_is_default = is_default;
}

void icon_button::set_focusable(bool focusable) {
  m_focusable = focusable;
  if(m_focusable) {
    setFocusPolicy(Qt::TabFocus);
  }
}

void icon_button::focusInEvent(QFocusEvent* event) {
  if(m_focusable) {
    m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
  }
}

void icon_button::focusOutEvent(QFocusEvent* event) {
  if(m_focusable) {
    m_label->setPixmap(QPixmap::fromImage(m_default_icon));
  }
}

void icon_button::leaveEvent(QEvent* event) {
  m_label->setPixmap(QPixmap::fromImage(m_default_icon));
}

void icon_button::mousePressEvent(QMouseEvent* event) {
  event->accept();
}

void icon_button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && m_clickable) {
    if(rect().contains(event->localPos().toPoint())) {
      m_clicked_signal();
    }
  }
}

void icon_button::set_clickable(bool clickable) {
  m_clickable = clickable;
}

void icon_button::swap_icons() {
  if(m_is_default) {
    m_label->setPixmap(QPixmap::fromImage(m_default_icon));
  } else {
    m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
  }
  m_is_default = !m_is_default;
}
