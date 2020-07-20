#include "Spire/Ui/ColorSelectorButton.hpp"
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Ui/DropDownWindow.hpp"

using namespace boost::signals2;
using namespace Spire;

ColorSelectorButton::ColorSelectorButton(const QColor& current_color,
    QWidget* parent)
    : QWidget(parent) {
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_Hover);
  m_selector_widget = new ColorSelectorDropDown(current_color, this);
  m_selector_widget->connect_color_signal([=] (const auto& color) {
    on_color_selected(color);
  });
  auto dropdown = new DropDownWindow(true, this);
  dropdown->set_widget(m_selector_widget);
  set_color(current_color);
}

const QColor& ColorSelectorButton::get_color() const {
  return m_current_color;
}

void ColorSelectorButton::set_color(const QColor& color) {
  m_current_color = color;
  m_selector_widget->set_color(color);
  update();
}

connection ColorSelectorButton::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

void ColorSelectorButton::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  auto painter = QPainter(this);
  if(hasFocus() || m_selector_widget->isActiveWindow() || underMouse()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.setPen(Qt::white);
  painter.drawRect(1, 1, width() - 3, height() - 3);
  painter.fillRect(2, 2, width() - 4, height() - 4, m_current_color);
}

void ColorSelectorButton::on_color_selected(const QColor& color) {
  m_current_color = color;
  m_color_signal(m_current_color);
  update();
}
