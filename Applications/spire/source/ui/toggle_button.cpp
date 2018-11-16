#include "spire/ui/toggle_button.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include "spire/spire/dimensions.hpp"

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
      m_icon_button(icon, hover_icon, icon,
        parent),
      m_icon(icon),
      m_hover_icon(hover_icon),
      m_toggle_icon(std::move(toggle_icon)),
      m_disabled_icon(std::move(disable_icon)) {
  setFocusPolicy(Qt::TabFocus);
  m_icon_button.installEventFilter(this);
  m_icon_button.setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(&m_icon_button);
}

void ToggleButton::setEnabled(bool enabled) {
  set_icons(enabled);
  QWidget::setEnabled(enabled);
}

void ToggleButton::setDisabled(bool disabled) {
  set_icons(!disabled);
  QWidget::setDisabled(disabled);
}

bool ToggleButton::eventFilter(QObject* object, QEvent* event) {
  if(event->type() == QEvent::WindowActivate ||
      event->type() == QEvent::WindowDeactivate) {
    return true;
  } else if(event->type() == QEvent::MouseButtonRelease) {
    auto e = static_cast<QMouseEvent*>(event);
    if(e->button() == Qt::LeftButton) {
      swap_toggle();
    }
  }
  return QWidget::eventFilter(object, event);
}

void ToggleButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    swap_toggle();
  } else {
    parent()->event(event);
  }
}

void ToggleButton::paintEvent(QPaintEvent* event) {
  if(hasFocus()) {
    auto painter = QPainter(this);
    painter.setPen(QColor("#4B23A0"));
    painter.drawLine(scale_width(5), event->rect().height() - 2,
      event->rect().width() - scale_width(6), event->rect().height() - 2);
    painter.drawLine(scale_width(5), event->rect().height() - 1,
      event->rect().width() - scale_width(6), event->rect().height() - 1);
  }
  QWidget::paintEvent(event);
}

void ToggleButton::resizeEvent(QResizeEvent* event) {
  m_icon_button.resize(size());
  QWidget::resizeEvent(event);
}

void ToggleButton::set_icons(bool enabled) {
  if(enabled) {
    if(m_is_toggled) {
      m_icon_button.set_icon(m_toggle_icon, m_hover_icon);
    } else {
      m_icon_button.set_icon(m_icon, m_hover_icon);
    }
  } else {
    m_icon_button.set_icon(m_disabled_icon, m_disabled_icon);
  }
}

void ToggleButton::swap_toggle() {
  if(m_is_toggled) {
    m_icon_button.set_icon(m_toggle_icon, m_toggle_icon);
  } else {
    m_icon_button.set_icon(m_icon, m_hover_icon);
  }
  m_is_toggled = !m_is_toggled;
}
