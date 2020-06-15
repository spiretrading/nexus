#include "Spire/Ui/ColorSelectorDropDown.hpp"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

namespace {

  auto BUTTON_HEIGHT() {
    static auto height = scale_height(20);
    return height;
  }

  auto BUTTON_WIDTH() {
    static auto width = scale_width(20);
    return width;
  }

  auto FONT_HEIGHT() {
    static auto height = scale_height(12);
    return height;
  }

  auto HORIZONTAL_PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }

  auto LABEL_HEIGHT() {
    static auto height = scale_height(14);
    return height;
  }

  auto SLIDER_HEIGHT() {
    static auto height = scale_height(20);
    return height;
  }

  auto SLIDER_WIDTH() {
    static auto width = scale_width(122);
    return width;
  }

  auto VERTICAL_PADDING() {
    static auto padding = scale_height(8);
    return padding;
  }

  auto create_color_button(const QColor& color, QWidget* parent) {
    auto button = new FlatButton(parent);
    button->setFixedSize(BUTTON_WIDTH(), BUTTON_HEIGHT());
    auto style = button->get_style();
    style.m_background_color = color;
    style.m_border_color = QColor("#C8C8C8");
    button->set_style(style);
    style.m_border_color = QColor("#4B23A0");
    button->set_hover_style(style);
    button->set_focus_style(style);
    return button;
  }

  void add_basic_color_button(QGridLayout* layout, int x, int y,
      const QColor& color, QWidget* parent) {
    auto button = create_color_button(color, parent);
    // TODO: connect clicked signal
    layout->addWidget(button, x, y);
  }

  void add_recent_color_button(QHBoxLayout* layout, const QColor& color,
      QWidget* parent) {
    auto button = create_color_button(color, parent);
    // TODO: connect clicked signal
    layout->addWidget(button);
  }
}

ColorSelectorDropDown::ColorSelectorDropDown(const QColor& current_color,
    QWidget* parent)
    : QWidget(parent) {
  setFixedSize(scale(232, 182));
  setStyleSheet("background-color: #FFFFFF;");
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(HORIZONTAL_PADDING(), VERTICAL_PADDING(),
    HORIZONTAL_PADDING(), VERTICAL_PADDING());
  layout->setSpacing(0);
  auto horizontal_label_layout = new QHBoxLayout();
  horizontal_label_layout->setContentsMargins({});
  horizontal_label_layout->setSpacing(scale_width(18));
  layout->addLayout(horizontal_label_layout);
  auto label_style = QString(R"(
      QLabel {
        font-family: Roboto;
        font-size: %1px;
      }
    )").arg(FONT_HEIGHT());
  auto basic_colors_label = new QLabel(tr("Basic Colors"), this);
  basic_colors_label->setStyleSheet(label_style);
  horizontal_label_layout->addWidget(basic_colors_label);
  horizontal_label_layout->addStretch(1);
  auto color_picker_label = new QLabel(tr("Color Picker"), this);
  color_picker_label->setFixedSize(scale_width(122), LABEL_HEIGHT());
  color_picker_label->setStyleSheet(label_style);
  horizontal_label_layout->addWidget(color_picker_label);
  auto horizontal_color_layout = new QHBoxLayout();
  horizontal_color_layout->setContentsMargins({});
  horizontal_color_layout->setSpacing(0);
  layout->addLayout(horizontal_color_layout);
  auto basic_colors_layout = new QGridLayout();
  basic_colors_layout->setContentsMargins(0, scale_height(10), 0,
    scale_height(9));
  basic_colors_layout->setHorizontalSpacing(HORIZONTAL_PADDING());
  basic_colors_layout->setVerticalSpacing(VERTICAL_PADDING());
  horizontal_color_layout->addLayout(basic_colors_layout);
  add_basic_color_button(basic_colors_layout, 0, 0, QColor("#F50000"), this);
  add_basic_color_button(basic_colors_layout, 0, 1, QColor("#FF8900"), this);
  add_basic_color_button(basic_colors_layout, 0, 2, QColor("#FFE200"), this);
  add_basic_color_button(basic_colors_layout, 1, 0, QColor("#2BE757"), this);
  add_basic_color_button(basic_colors_layout, 1, 1, QColor("#0065FD"), this);
  add_basic_color_button(basic_colors_layout, 1, 2, QColor("#BA19E9"), this);
  add_basic_color_button(basic_colors_layout, 2, 0, QColor("#FF00CC"), this);
  add_basic_color_button(basic_colors_layout, 2, 1, QColor("#FFFFFF"), this);
  add_basic_color_button(basic_colors_layout, 2, 2, QColor("#000000"), this);
  horizontal_color_layout->addStretch(1);
  auto color_picker_layout = new QVBoxLayout();
  color_picker_layout->setContentsMargins(0, scale_height(10), 0, 0);
  color_picker_layout->setSpacing(VERTICAL_PADDING());
  horizontal_color_layout->addLayout(color_picker_layout);
  m_color_value_slider = new ColorSelectorSlider(
    ColorSelectorSlider::SliderMode::VALUE, current_color, this);
  m_color_value_slider->setFixedSize(scale(SLIDER_WIDTH(), SLIDER_HEIGHT()));
  m_color_value_slider->connect_color_signal([=] (const auto& color) {
    on_color_selected(color);
  });
  color_picker_layout->addWidget(m_color_value_slider);
  m_color_hue_slider = new ColorSelectorSlider(
    ColorSelectorSlider::SliderMode::HUE, current_color, this);
  m_color_hue_slider->setFixedSize(scale(SLIDER_WIDTH(), SLIDER_HEIGHT()));
  m_color_hue_slider->connect_color_signal([=] (const auto& color) {
    on_color_selected(color);
  });
  color_picker_layout->addWidget(m_color_hue_slider);
  m_hex_input = new ColorSelectorHexInputWidget(current_color, this);
  m_hex_input->setFixedSize(scale(122, 26));
  m_hex_input->connect_color_signal([=] (const auto& color) {
    on_color_selected(color); });
  color_picker_layout->addWidget(m_hex_input);
  color_picker_layout->addStretch(1);
  layout->addSpacing(scale_height(12));
  auto recent_colors_label = new QLabel(tr("Recent Colors"), this);
  recent_colors_label->setFixedHeight(scale_height(LABEL_HEIGHT()));
  recent_colors_label->setStyleSheet(label_style);
  layout->addWidget(recent_colors_label);
  auto recent_colors_layout = new QHBoxLayout();
  recent_colors_layout->setContentsMargins(0, scale_height(10), 0, 0);
  recent_colors_layout->setSpacing(HORIZONTAL_PADDING());
  layout->addLayout(recent_colors_layout);
  add_recent_color_button(recent_colors_layout, QColor("#4b23a0"), this);
  add_recent_color_button(recent_colors_layout, QColor("#4b23a0"), this);
  add_recent_color_button(recent_colors_layout, QColor("#4b23a0"), this);
  add_recent_color_button(recent_colors_layout, QColor("#4b23a0"), this);
  add_recent_color_button(recent_colors_layout, QColor("#4b23a0"), this);
  add_recent_color_button(recent_colors_layout, QColor("#4b23a0"), this);
  add_recent_color_button(recent_colors_layout, QColor("#4b23a0"), this);
  add_recent_color_button(recent_colors_layout, QColor("#4b23a0"), this);
}

void ColorSelectorDropDown::on_color_selected(const QColor& color) {
  m_color_value_slider->set_color(color);
  m_color_hue_slider->set_color(color);
  m_hex_input->set_color(color);
}
