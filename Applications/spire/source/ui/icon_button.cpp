#include "spire/ui/icon_button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

using namespace boost;
using namespace boost::signals2;
using namespace spire;

icon_button::icon_button(QImage icon, QWidget* parent)
    : icon_button(icon, icon, parent) {}

icon_button::icon_button(QImage icon, QImage hover_icon, QWidget* parent)
    : icon_button(icon, hover_icon, icon, parent) {}

icon_button::icon_button(QImage icon, QImage hover_icon, QImage blur_icon,
    QWidget* parent)
    : QAbstractButton(parent),
      m_icon(std::move(icon)),
      m_hover_icon(std::move(hover_icon)),
      m_blur_icon(std::move(blur_icon)) {
  setFocusPolicy(Qt::StrongFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  setFixedSize(m_icon.size());
  show_normal();
}

void icon_button::set_default_style(const QString& stylesheet) {
  m_default_stylesheet = stylesheet;
}

void icon_button::set_hover_style(const QString& stylesheet) {
  m_hover_stylesheet = stylesheet;
}

const QImage& icon_button::get_icon() const {
  return m_icon;
}

void icon_button::set_icon(QImage icon) {
  set_icon(icon, icon);
}

void icon_button::set_icon(QImage icon, QImage hover_icon) {
  set_icon(icon, hover_icon, icon);
}

void icon_button::set_icon(QImage icon, QImage hover_icon, QImage blur_icon) {
  m_icon = std::move(icon);
  m_hover_icon = std::move(hover_icon);
  m_blur_icon = std::move(blur_icon);
  setFixedSize(m_icon.size());
  update();
}

connection icon_button::connect_clicked_signal(
    const clicked_signal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

#include <QDebug>

void icon_button::enterEvent(QEvent* event) {
  qDebug() << "enter " << this;
  if(isEnabled()) {
    switch(m_state) {
      case state::NORMAL:
        return show_hovered();
      case state::BLURRED:
        return show_hover_blurred();
    }
  }
}

void icon_button::focusInEvent(QFocusEvent* event) {
  if(focusPolicy() & Qt::TabFocus) {
    switch(m_state) {
      case state::NORMAL:
        return show_hovered();
      case state::BLURRED:
        return show_hover_blurred();
    }
  }
}

void icon_button::focusOutEvent(QFocusEvent* event) {
  if(focusPolicy() & Qt::TabFocus) {
    switch(m_state) {
      case state::HOVERED:
        return show_normal();
      case state::HOVER_BLURRED:
        return show_blurred();
    }
  }
}

void icon_button::leaveEvent(QEvent* event) {
  switch(m_state) {
    case state::HOVERED:
      return show_normal();
    case state::HOVER_BLURRED:
      return show_blurred();
  }
}

void icon_button::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    event->accept();
  }
}

void icon_button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(rect().contains(event->localPos().toPoint())) {
      event->accept();
      m_clicked_signal();
    }
  }
}

void icon_button::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  QStyleOption style_option;
  style_option.initFrom(this);
  style()->drawPrimitive(QStyle::PE_Widget, &style_option, &painter, this);
  if(m_state == state::NORMAL) {
    painter.drawImage(0, 0, m_icon);
  } else if(m_state == state::BLURRED) {
    painter.drawImage(0, 0, m_blur_icon);
  } else {
    painter.drawImage(0, 0, m_hover_icon);
  }
}

bool icon_button::event(QEvent* event) {
  if(event->type() == QEvent::WindowDeactivate) {
    switch(m_state) {
      case state::NORMAL:
        show_blurred();
        break;
      case state::HOVERED:
        show_blurred();
        break;
    }
  } else if(event->type() == QEvent::WindowActivate) {
    switch(m_state) {
      case state::BLURRED:
        show_normal();
        break;
      case state::HOVER_BLURRED:
        show_hovered();
        break;
    }
  }
  return QWidget::event(event);
}

void icon_button::show_normal() {
  m_state = state::NORMAL;
  setStyleSheet(m_default_stylesheet);
  update();
}

void icon_button::show_hovered() {
  m_state = state::HOVERED;
  setStyleSheet(m_hover_stylesheet);
  update();
}

void icon_button::show_blurred() {
  m_state = state::BLURRED;
  setStyleSheet(m_default_stylesheet);
  update();
}

void icon_button::show_hover_blurred() {
  m_state = state::HOVER_BLURRED;
  setStyleSheet(m_hover_stylesheet);
  update();
}
