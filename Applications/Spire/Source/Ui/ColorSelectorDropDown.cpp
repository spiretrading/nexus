#include "Spire/Ui/ColorSelectorDropDown.hpp"
#include <QChildEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace boost::signals2;
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
}

ColorSelectorDropDown::ColorSelectorDropDown(const QColor& current_color,
    const RecentColors& recent_colors, QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_current_color(current_color) {
  m_drop_shadow = new DropShadow(true, false, this);
  setFixedSize(scale(232, 198));
  setObjectName("color_selector_drop_down");
  setStyleSheet(R"(
    #color_selector_drop_down {
      background-color: #FFFFFF;
      border: 1px solid #C8C8C8;
    })");
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
        background-color: #FFFFFF;
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
  m_basic_colors_layout = new QGridLayout();
  m_basic_colors_layout->setContentsMargins(0, scale_height(10), 0,
    scale_height(36));
  m_basic_colors_layout->setHorizontalSpacing(HORIZONTAL_PADDING());
  m_basic_colors_layout->setVerticalSpacing(VERTICAL_PADDING());
  horizontal_color_layout->addLayout(m_basic_colors_layout);
  add_basic_color_button(0, 0, QColor("#F50000"));
  add_basic_color_button(0, 1, QColor("#FF8900"));
  add_basic_color_button(0, 2, QColor("#FFE200"));
  add_basic_color_button(1, 0, QColor("#2BE757"));
  add_basic_color_button(1, 1, QColor("#0065FD"));
  add_basic_color_button(1, 2, QColor("#BA19E9"));
  add_basic_color_button(2, 0, QColor("#FF00CC"));
  add_basic_color_button(2, 1, QColor("#FFFFFF"));
  add_basic_color_button(2, 2, QColor("#000000"));
  horizontal_color_layout->addStretch(1);
  auto color_picker_layout = new QVBoxLayout();
  color_picker_layout->setContentsMargins(0, scale_height(10), 0, 0);
  color_picker_layout->setSpacing(VERTICAL_PADDING());
  horizontal_color_layout->addLayout(color_picker_layout);
  m_color_value_slider = new ColorSelectorValueSlider(m_current_color, this);
  m_color_value_slider->setFocusPolicy(Qt::NoFocus);
  m_color_value_slider->setFixedSize(scale(SLIDER_WIDTH(), scale_height(48)));
  m_color_value_slider->connect_color_signal([=] (const auto& color) {
    m_hex_input->set_color(color);
    on_color_selected(color);
  });
  color_picker_layout->addWidget(m_color_value_slider);
  m_color_hue_slider = new ColorSelectorHueSlider(m_current_color, this);
  m_color_hue_slider->setFocusPolicy(Qt::NoFocus);
  m_color_hue_slider->setFixedSize(scale(SLIDER_WIDTH(), SLIDER_HEIGHT()));
  m_color_hue_slider->connect_color_signal([=] (const auto& color) {
    m_color_value_slider->set_hue(color.hue());
    m_hex_input->set_color(color);
    on_color_selected(color);
  });
  color_picker_layout->addWidget(m_color_hue_slider);
  m_hex_input = new ColorSelectorHexInputWidget(m_current_color, this);
  m_hex_input->setFixedSize(scale(122, 26));
  m_hex_input->connect_color_signal([=] (const auto& color) {
    m_color_value_slider->set_color(color);
    m_color_hue_slider->set_color(color);
    on_color_selected(color);
  });
  color_picker_layout->addWidget(m_hex_input);
  color_picker_layout->addStretch(1);
  auto recent_colors_label = new QLabel(tr("Recent Colors"), this);
  recent_colors_label->setFixedHeight(scale_height(LABEL_HEIGHT()));
  recent_colors_label->setStyleSheet(label_style);
  layout->addWidget(recent_colors_label);
  m_recent_colors_layout = new QHBoxLayout();
  m_recent_colors_layout->setContentsMargins(0, scale_height(10), 0, 0);
  m_recent_colors_layout->setSpacing(HORIZONTAL_PADDING());
  layout->addLayout(m_recent_colors_layout);
  for(const auto& recent_color : recent_colors.get_colors()) {
    add_recent_color_button(recent_color);
  }
}

void ColorSelectorDropDown::set_color(const QColor& color) {
  m_color_hue_slider->set_color(color);
  m_color_value_slider->set_color(color);
  m_hex_input->set_color(color);
}

connection ColorSelectorDropDown::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

void ColorSelectorDropDown::childEvent(QChildEvent* event) {
  if(event->added()) {
    event->child()->installEventFilter(this);
  }
}
bool ColorSelectorDropDown::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Escape) {
      hide();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ColorSelectorDropDown::add_basic_color_button(int x, int y,
    const QColor& color) {
  auto button = create_color_button(color, this);
  button->connect_clicked_signal([=, color = std::move(color)] {
    on_color_button_clicked(color);
  });
  m_basic_colors_layout->addWidget(button, x, y);
}

void ColorSelectorDropDown::add_recent_color_button(const QColor& color) {
  auto button = create_color_button(color, this);
  button->connect_clicked_signal([=, color = std::move(color)] {
    on_color_button_clicked(color);
  });
  m_recent_colors_layout->addWidget(button);
}

void ColorSelectorDropDown::on_color_button_clicked(const QColor& color) {
  set_color(color);
  on_color_selected(color);
}

void ColorSelectorDropDown::on_color_selected(const QColor& color) {
  m_current_color = color;
  m_color_signal(m_current_color);
}
