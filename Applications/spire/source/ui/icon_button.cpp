#include "spire/ui/icon_button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

IconButton::IconButton(QImage icon, QWidget* parent)
    : IconButton(icon, icon, parent) {}

IconButton::IconButton(QImage icon, QImage hover_icon, QWidget* parent)
    : IconButton(icon, hover_icon, icon, parent) {}

IconButton::IconButton(QImage icon, QImage hover_icon,
    QImage blur_icon, QWidget* parent)
    : QAbstractButton(parent),
      m_icon(std::move(icon)),
      m_hover_icon(std::move(hover_icon)),
      m_blur_icon(std::move(blur_icon)) {
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  setFixedSize(m_icon.size());
  show_normal();
}

void IconButton::set_default_style(const QString& stylesheet) {
  m_default_stylesheet = stylesheet;
}

void IconButton::set_hover_style(const QString& stylesheet) {
  m_hover_stylesheet = stylesheet;
}

const QImage& IconButton::get_icon() const {
  return m_icon;
}

void IconButton::set_icon(QImage icon) {
  set_icon(icon, icon);
}

void IconButton::set_icon(QImage icon, QImage hover_icon) {
  set_icon(icon, hover_icon, icon);
}

void IconButton::set_icon(QImage icon, QImage hover_icon, QImage blur_icon) {
  m_icon = std::move(icon);
  m_hover_icon = std::move(hover_icon);
  m_blur_icon = std::move(blur_icon);
  setFixedSize(m_icon.size());
  update();
}

connection IconButton::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void IconButton::focusInEvent(QFocusEvent* event) {
  if(focusPolicy() & Qt::TabFocus) {
    switch(m_state) {
      case State::NORMAL:
        return show_hovered();
      case State::BLURRED:
        return show_hover_blurred();
    }
  }
}

void IconButton::focusOutEvent(QFocusEvent* event) {
  if(focusPolicy() & Qt::TabFocus) {
    switch(m_state) {
      case State::HOVERED:
        return show_normal();
      case State::HOVER_BLURRED:
        return show_blurred();
    }
  }
}

void IconButton::hideEvent(QHideEvent* event) {
  if(m_state == State::HOVERED) {
    show_normal();
  }
}

void IconButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    m_clicked_signal();
    event->accept();
    return;
  }
  event->ignore();
}

void IconButton::leaveEvent(QEvent* event) {
  switch(m_state) {
    case State::HOVERED:
      return show_normal();
    case State::HOVER_BLURRED:
      return show_blurred();
  }
}

void IconButton::mouseMoveEvent(QMouseEvent* event) {
  if(isEnabled() && m_state != State::BLURRED &&
      m_state != State::HOVER_BLURRED) {
    switch(m_state) {
      case State::NORMAL:
        return show_hovered();
      case State::BLURRED:
        return show_hover_blurred();
    }
  }
}

void IconButton::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    event->accept();
  }
}

void IconButton::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(rect().contains(event->localPos().toPoint())) {
      event->accept();
      m_clicked_signal();
    }
  }
}

void IconButton::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  QStyleOption style_option;
  style_option.initFrom(this);
  style()->drawPrimitive(QStyle::PE_Widget, &style_option, &painter, this);
  if(m_state == State::NORMAL) {
    painter.drawImage(0, 0, m_icon);
  } else if(m_state == State::BLURRED) {
    painter.drawImage(0, 0, m_blur_icon);
  } else {
    painter.drawImage(0, 0, m_hover_icon);
  }
}

bool IconButton::event(QEvent* event) {
  if(event->type() == QEvent::WindowDeactivate) {
    switch(m_state) {
      case State::NORMAL:
        show_blurred();
        break;
      case State::HOVERED:
        show_blurred();
        break;
    }
  } else if(event->type() == QEvent::WindowActivate) {
    switch(m_state) {
      case State::BLURRED:
        show_normal();
        break;
      case State::HOVER_BLURRED:
        show_hovered();
        break;
    }
  }
  return QWidget::event(event);
}

void IconButton::show_normal() {
  m_state = State::NORMAL;
  setStyleSheet(m_default_stylesheet);
  update();
}

void IconButton::show_hovered() {
  m_state = State::HOVERED;
  setStyleSheet(m_hover_stylesheet);
  update();
}

void IconButton::show_blurred() {
  m_state = State::BLURRED;
  setStyleSheet(m_default_stylesheet);
  update();
}

void IconButton::show_hover_blurred() {
  m_state = State::HOVER_BLURRED;
  setStyleSheet(m_hover_stylesheet);
  update();
}
