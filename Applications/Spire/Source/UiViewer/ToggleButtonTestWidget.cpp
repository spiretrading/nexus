#include "Spire/UiViewer/ToggleButtonTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"

using namespace Spire;

namespace {
  auto IMAGE_SIZE() {
    static auto size = scale(16, 16);
    return size;
  }

  auto BUTTON_SIZE() {
    static auto size = scale(16, 22);
    return size;
  }
}

ToggleButtonTestWidget::ToggleButtonTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  auto default_label = new QLabel(tr("Toggle Image"), this);
  m_layout->addWidget(default_label, 0, 0, 1, 3);
  auto default_button = new ToggleButton(
    imageFromSvg(":/Icons/lock-grid-light-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-green.svg", IMAGE_SIZE()), this);
  default_button->setFixedSize(BUTTON_SIZE());
  default_button->setToolTip(tr("Tooltip"));
  add_button(default_button, 1);
  auto hover_style_label = new QLabel(tr("Toggle and Hover Images"), this);
  m_layout->addWidget(hover_style_label, 3, 0, 1, 3);
  auto hover_style_button = new ToggleButton(
    imageFromSvg(":/Icons/lock-grid-light-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-green.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-purple.svg", IMAGE_SIZE()), this);
  hover_style_button->setFixedSize(BUTTON_SIZE());
  hover_style_button->setToolTip(tr("Tooltip"));
  add_button(hover_style_button, 4);
  auto disabled_style_label = new QLabel(
    tr("Default, Hover, and Disabled Images"), this);
  m_layout->addWidget(disabled_style_label, 6, 0, 1, 3);
  auto disabled_style_button = new ToggleButton(
    imageFromSvg(":/Icons/lock-grid-light-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-green.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-grey.svg", IMAGE_SIZE()), this);
  disabled_style_button->setFixedSize(BUTTON_SIZE());
  disabled_style_button->setToolTip(tr("Tooltip"));
  add_button(disabled_style_button, 7);
  m_pressed_label = new QLabel(tr("Button Pressed"), this);
  m_pressed_label->hide();
  m_pressed_timer = new QTimer(this);
  m_pressed_timer->setInterval(1000);
  m_pressed_timer->setSingleShot(true);
  connect(m_pressed_timer, &QTimer::timeout, [=] {
    m_pressed_label->hide();
  });
}

void ToggleButtonTestWidget::add_button(ToggleButton* button, int row) {
  button->setFixedSize(BUTTON_SIZE());
  auto toggled_check_box = make_check_box(tr("Toggled"), this);
  button->connect_clicked_signal([=] {
    toggled_check_box->setChecked(!toggled_check_box->isChecked());
    on_button_pressed(row);
  });
  m_layout->addWidget(button, row, 0);
  auto disable_check_box = make_check_box(tr("Disable"), this);
  connect(disable_check_box, &CheckBox::stateChanged, [=] (auto state) {
    button->setDisabled(disable_check_box->isChecked());
  });
  m_layout->addWidget(disable_check_box, row, 2);
  connect(toggled_check_box, &CheckBox::stateChanged, [=] (auto state) {
    button->set_toggled(toggled_check_box->isChecked());
  });
  m_layout->addWidget(toggled_check_box, row + 1, 2);
}

void ToggleButtonTestWidget::on_button_pressed(int row) {
  m_layout->removeWidget(m_pressed_label);
  m_layout->addWidget(m_pressed_label, row, 1);
  m_pressed_label->show();
  m_pressed_timer->start();
}
