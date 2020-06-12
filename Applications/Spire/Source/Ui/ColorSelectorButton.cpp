#include "Spire/Ui/ColorSelectorButton.hpp"
#include <QHBoxLayout>

using namespace Spire;

ColorSelectorButton::ColorSelectorButton(const QColor& color, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_button = new FlatButton(this);
  set_button_color(color);
  layout->addWidget(m_button);
  m_dropdown = new ColorSelectorButtonDropDown(this);
}

void ColorSelectorButton::set_button_color(const QColor& color) {
  auto style = m_button->get_style();
  style.m_background_color = color;
  style.m_border_color = QColor("#C8C8C8");
  m_button->set_style(style);
  style.m_border_color = QColor("#4B23A0");
  m_button->set_hover_style(style);
  m_button->set_focus_style(style);
}
