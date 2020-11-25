#include "Spire/UiViewer/ColorSelectorButtonTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

namespace {
  auto INPUT_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }
}

ColorSelectorButtonTestWidget::ColorSelectorButtonTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container_widget = new QWidget(this);
  m_layout = new QGridLayout(container_widget);
  m_color_selector_button = new ColorSelectorButton(QColor("#4B23A0"),
    this);
  m_color_selector_button->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(m_color_selector_button, 0, 0);
  auto color_selector_button_value = new QLabel(
    m_color_selector_button->get_color().name().toUpper(), this);
  m_layout->addWidget(color_selector_button_value, 0, 1);
  auto set_color_input = new TextInputWidget(this);
  set_color_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(set_color_input, 1, 0);
  auto set_color_button = make_flat_button(tr("Set Color"), this);
  set_color_button->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(set_color_button, 1, 1);
  auto create_color_input = new TextInputWidget(this);
  create_color_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(create_color_input, 2, 0);
  auto create_color_button = make_flat_button(tr("Replace Button"), this);
  create_color_button->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(create_color_button, 2, 1);
  m_color_selector_button->connect_color_signal([=] (const auto& color) {
    color_selector_button_value->setText(color.name().toUpper());
  });
  set_color_button->connect_clicked_signal([=] {
    on_set_color_button_color(set_color_input->text());
  });
  create_color_button->connect_clicked_signal([=] {
    on_create_color_button_color(create_color_input->text());
  });
}

void ColorSelectorButtonTestWidget::on_create_color_button_color(
    const QString& color_hex) {
  auto color = QColor(QString("#%1").arg(color_hex));
  if(color.isValid()) {
    delete_later(m_color_selector_button);
    m_color_selector_button = new ColorSelectorButton(color, this);
    m_color_selector_button->setFixedSize(INPUT_SIZE());
    m_layout->addWidget(m_color_selector_button, 0, 0);
  }
}

void ColorSelectorButtonTestWidget::on_set_color_button_color(
    const QString& color_hex) {
  auto color = QColor(QString("#%1").arg(color_hex));
  if(color.isValid()) {
    m_color_selector_button->set_color(color);
  }
}
