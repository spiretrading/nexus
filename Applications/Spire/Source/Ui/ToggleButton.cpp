#include "Spire/Ui/ToggleButton.hpp"
#include <QEvent>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

ToggleButton::ToggleButton(QImage icon, QImage toggle_icon, QWidget* parent)
    : ToggleButton(icon, toggle_icon, icon, parent) {}

ToggleButton::ToggleButton(QImage icon, QImage toggle_icon, QImage hover_icon,
    QWidget* parent)
    : ToggleButton(icon, toggle_icon, hover_icon, icon, parent) {}

ToggleButton::ToggleButton(QImage icon, QImage toggle_icon, QImage hover_icon,
    QImage disable_icon, QWidget* parent)
    : QWidget(parent),
      m_is_toggled(false),
      m_is_focused(false),
      m_icon_button(icon, hover_icon, icon,
        parent),
      m_icon(std::move(icon)),
      m_hover_icon(std::move(hover_icon)),
      m_toggle_icon(std::move(toggle_icon)),
      m_disabled_icon(std::move(disable_icon)) {
  setStyleSheet("background-color: red;");
  setFocusPolicy(Qt::NoFocus);
  m_icon_button.installEventFilter(this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(&m_icon_button);
}

void ToggleButton::set_toggled(bool toggled) {
  if(m_is_toggled != toggled) {
    swap_toggle();
  }
}

void ToggleButton::setEnabled(bool enabled) {
  set_icons(enabled);
  QWidget::setEnabled(enabled);
}

void ToggleButton::setDisabled(bool disabled) {
  set_icons(!disabled);
  QWidget::setDisabled(disabled);
}

connection ToggleButton::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_icon_button.connect_clicked_signal(slot);
}

bool ToggleButton::eventFilter(QObject* object, QEvent* event) {
  if(event->type() == QEvent::WindowActivate ||
      event->type() == QEvent::WindowDeactivate) {
    return true;
  } else if(event->type() == QEvent::MouseButtonRelease) {
    auto mouse_event = static_cast<QMouseEvent*>(event);
    if(mouse_event->button() == Qt::LeftButton) {
      if(isEnabled()) {
        swap_toggle();
      }
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return ||
        e->key() == Qt::Key_Space) {
      if(isEnabled()) {
        swap_toggle();
      }
    }
  } else if(event->type() == QEvent::FocusIn) {
    auto e = static_cast<QFocusEvent*>(event);
    if(e->reason() == Qt::TabFocusReason ||
        e->reason() == Qt::BacktabFocusReason) {
      m_is_focused = true;
      update();
    }
  } else if(event->type() == QEvent::FocusOut) {
    m_is_focused = false;
    update();
  }
  return QWidget::eventFilter(object, event);
}

void ToggleButton::focusInEvent(QFocusEvent* event) {
  if(event->reason() == Qt::TabFocusReason ||
      event->reason() == Qt::BacktabFocusReason) {
    m_is_focused = true;
    update();
  }
}

void ToggleButton::focusOutEvent(QFocusEvent* event) {
  m_is_focused = false;
  update();
}

void ToggleButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    if(isEnabled()) {
      swap_toggle();
    }
    event->accept();
    return;
  }
  event->ignore();
}

void ToggleButton::resizeEvent(QResizeEvent* event) {
  m_icon_button.resize(size());
  QWidget::resizeEvent(event);
}

void ToggleButton::set_icons(bool enabled) {
  if(enabled) {
    if(!isEnabled() && m_is_toggled) {
      m_icon_button.set_icon(m_toggle_icon, m_toggle_icon);
    } else if(m_is_toggled) {
      m_icon_button.set_icon(m_toggle_icon, m_hover_icon);
    } else {
      m_icon_button.set_icon(m_icon, m_hover_icon);
    }
  } else {
    m_icon_button.set_icon(m_disabled_icon, m_disabled_icon);
  }
}

void ToggleButton::swap_toggle() {
  m_is_toggled = !m_is_toggled;
  if(m_is_toggled) {
    m_icon_button.set_icon(m_toggle_icon, m_toggle_icon);
  } else {
    m_icon_button.set_icon(m_icon, m_hover_icon);
  }
}
