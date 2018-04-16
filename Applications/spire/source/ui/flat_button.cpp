#include "spire/ui/flat_button.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointF>

using namespace boost;
using namespace boost::signals2;
using namespace spire;

flat_button::flat_button(QWidget* parent)
  : flat_button("", parent) {}

flat_button::flat_button(const QString& label, QWidget* parent)
    : QWidget(parent),
      m_clickable(true),
      m_last_focus_reason(Qt::TabFocusReason) {
  m_label = new QLabel(label, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_label);
  setFocusPolicy(Qt::StrongFocus);
}

void flat_button::set_text(const QString& text) {
  m_label->setText(text);
}

void flat_button::set_stylesheet(const QString& default_style,
    const QString& hover_style, const QString& focused_style,
    const QString& disabled_style) {
  m_default_style = QString("QLabel { %1 }").arg(default_style);
  m_hover_style = QString("QLabel:hover { %1 }").arg(hover_style);
  m_focused_style = QString("QLabel { %1 }").arg(focused_style);
  m_disabled_style = QString("QLabel { %1 }").arg(disabled_style);
  set_hover_stylesheet();
}

connection flat_button::connect_clicked_signal(
    const clicked_signal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void flat_button::changeEvent(QEvent* event) {
  if(event->type() == QEvent::FontChange) {
    m_label->setFont(font());
  } else if(event->type() == QEvent::EnabledChange) {
    if(!isEnabled()) {
      disable_button();
    } else {
      enable_button();
    }
  }
}

void flat_button::focusInEvent(QFocusEvent* event) {
  if(event->reason() == Qt::ActiveWindowFocusReason) {
    if(m_last_focus_reason == Qt::MouseFocusReason) {
      set_hover_stylesheet();
    } else {
      set_focused_stylesheet();
    }
    return;
  } else if(event->reason() != Qt::MouseFocusReason) {
    set_focused_stylesheet();
  } else {
    set_hover_stylesheet();
  }
  m_last_focus_reason = event->reason();
}

void flat_button::focusOutEvent(QFocusEvent* event) {
  set_hover_stylesheet();
}

void flat_button::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(m_clickable) {
      m_clicked_signal();
    }
  }
}

void flat_button::mousePressEvent(QMouseEvent* event) {
  event->accept();
}

void flat_button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && m_clickable) {
    set_hover_stylesheet();
    m_last_focus_reason = Qt::MouseFocusReason;
    if(rect().contains(event->localPos().toPoint())) {
      m_clicked_signal();
    }
  }
}

void flat_button::disable_button() {
  m_clickable = false;
  setFocusPolicy(Qt::NoFocus);
  set_disabled_stylesheet();
}

void flat_button::enable_button() {
  m_clickable = true;
  setFocusPolicy(Qt::StrongFocus);
  set_hover_stylesheet();
}

void flat_button::set_disabled_stylesheet() {
  setStyleSheet(m_disabled_style);
}

void flat_button::set_focused_stylesheet() {
  setStyleSheet(m_default_style + m_hover_style + m_focused_style);
}

void flat_button::set_hover_stylesheet() {
  setStyleSheet(m_default_style + m_hover_style);
}
