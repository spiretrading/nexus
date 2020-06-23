#include "Spire/Ui/ColorSelectorButton.hpp"
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>

using namespace boost::signals2;
using namespace Spire;

ColorSelectorButton::ColorSelectorButton(const QColor& current_color,
    const RecentColors& recent_colors, QWidget* parent)
    : QWidget(parent),
      m_recent_colors(recent_colors) {
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_Hover);
  m_dropdown = new ColorSelectorDropDown(current_color, recent_colors, this);
  m_dropdown->connect_color_signal([=] (const auto& color) {
    on_color_selected(color);
  });
  m_dropdown->hide();
  window()->installEventFilter(this);
  set_color(current_color);
}

const QColor& ColorSelectorButton::get_color() const {
  return m_current_color;
}

void ColorSelectorButton::set_color(const QColor& color) {
  m_current_color = color;
  m_dropdown->set_color(color);
  update();
}

void ColorSelectorButton::set_recent_colors(
    const RecentColors& recent_colors) {
  m_dropdown->set_recent_colors(recent_colors);
}

connection ColorSelectorButton::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

connection ColorSelectorButton::connect_recent_colors_signal(
    const RecentColorsSignal::slot_type& slot) const {
  return m_recent_colors_signal.connect(slot);
}

bool ColorSelectorButton::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move) {
      if(m_dropdown->isVisible()) {
        move_color_dropdown();
      }
    } else if(event->type() == QEvent::WindowDeactivate &&
        !m_dropdown->isActiveWindow()) {
      hide_dropdown();
    } else if(event->type() == QEvent::MouseButtonPress) {
      hide_dropdown();
    }
  } else if(watched == m_dropdown) {
    if(event->type() == QEvent::Hide) {
      hide_dropdown();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ColorSelectorButton::focusOutEvent(QFocusEvent* event) {
  if(!m_dropdown->isActiveWindow()) {
    hide_dropdown();
  }
  update();
}

void ColorSelectorButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    if(m_dropdown->isVisible()) {
      m_dropdown->hide();
    }
  }
}

void ColorSelectorButton::mousePressEvent(QMouseEvent* event) {
  if(m_dropdown->isVisible()) {
    hide_dropdown();
  } else {
    move_color_dropdown();
    m_dropdown->show();
    m_dropdown->activateWindow();
  }
}

void ColorSelectorButton::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.fillRect(0, 0, 1000, 1000, Qt::red);
  if(hasFocus() || m_dropdown->isActiveWindow() || underMouse()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.setPen(Qt::white);
  painter.drawRect(1, 1, width() - 3, height() - 3);
  painter.fillRect(2, 2, width() - 4, height() - 4, m_current_color);
}

void ColorSelectorButton::hide_dropdown() {
  m_dropdown->hide();
  if(m_dropdown->get_recent_colors() != m_recent_colors) {
    m_recent_colors = m_dropdown->get_recent_colors();
    m_recent_colors_signal(m_recent_colors);
  }
}

void ColorSelectorButton::move_color_dropdown() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_dropdown->move(x_pos, y_pos + 1);
  m_dropdown->raise();
}

void ColorSelectorButton::on_color_selected(const QColor& color) {
  m_current_color = color;
  m_color_signal(m_current_color);
  update();
}
