#include "Spire/UiViewer/ToggleButtonTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ToggleButton.hpp"

using namespace Spire;

namespace {
  auto IMAGE_SIZE() {
    static auto size = scale(16, 16);
    return size;
  }

  auto BUTTON_SIZE() {
    static auto size = scale(16, 26);
    return size;
  }
}

ToggleButtonTestWidget::ToggleButtonTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  auto label1 = new QLabel(tr("Toggle Image"), this);
  m_layout->addWidget(label1, 0, 0, 1, 3);
  auto button1 = new ToggleButton(
    imageFromSvg(":/Icons/lock-grid-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-green.svg", IMAGE_SIZE()), this);
  button1->setFixedSize(BUTTON_SIZE());
  auto toggled_check_box1 = make_check_box(tr("Toggled"), this);
  button1->connect_clicked_signal([=] {
    toggled_check_box1->setChecked(!toggled_check_box1->isChecked());
    on_button_pressed(1);
  });
  m_layout->addWidget(button1, 1, 0);
  auto disable_check_box1 = make_check_box(tr("Disable"), this);
  connect(disable_check_box1, &CheckBox::stateChanged, [=] (auto state) {
    button1->setDisabled(disable_check_box1->isChecked());
  });
  m_layout->addWidget(disable_check_box1, 1, 2);
  connect(toggled_check_box1, &CheckBox::stateChanged, [=] (auto state) {
    button1->set_toggled(toggled_check_box1->isChecked());
  });
  m_layout->addWidget(toggled_check_box1, 2, 2);
  auto label2 = new QLabel(tr("Toggle and Hover Images"), this);
  m_layout->addWidget(label2, 3, 0, 1, 3);
  auto button2 = new ToggleButton(
    imageFromSvg(":/Icons/lock-grid-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-green.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-purple.svg", IMAGE_SIZE()), this);
  button2->setFixedSize(BUTTON_SIZE());
  auto toggled_check_box2 = make_check_box(tr("Toggled"), this);
  button2->connect_clicked_signal([=] {
    toggled_check_box2->setChecked(!toggled_check_box2->isChecked());
    on_button_pressed(4);
  });
  m_layout->addWidget(button2, 4, 0);
  connect(toggled_check_box2, &CheckBox::stateChanged, [=] (auto state) {
    button2->set_toggled(toggled_check_box2->isChecked());
  });
  m_layout->addWidget(toggled_check_box2, 5, 2);
  auto disable_check_box2 = make_check_box(tr("Disable"), this);
  connect(disable_check_box2, &CheckBox::stateChanged, [=] (auto state) {
    button2->setDisabled(disable_check_box2->isChecked());
  });
  m_layout->addWidget(disable_check_box2, 4, 2);
  auto label3 = new QLabel(
    tr("Default, Hover, and Disabled Images"), this);
  m_layout->addWidget(label3, 6, 0, 1, 3);
  auto button3 = new ToggleButton(
    imageFromSvg(":/Icons/lock-grid-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-green.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/lock-grid-grey.svg", IMAGE_SIZE()), this);
  button3->setFixedSize(BUTTON_SIZE());
  auto toggled_check_box3 = make_check_box(tr("Toggled"), this);
  button3->connect_clicked_signal([=] {
    toggled_check_box3->setChecked(!toggled_check_box3->isChecked());
    on_button_pressed(7);
  });
  m_layout->addWidget(button3, 7, 0);
  auto disable_check_box3 = make_check_box(tr("Disable"), this);
  connect(disable_check_box3, &CheckBox::stateChanged, [=] (auto state) {
    button3->setDisabled(disable_check_box3->isChecked());
  });
  m_layout->addWidget(disable_check_box3, 7, 2);
  connect(toggled_check_box3, &CheckBox::stateChanged, [=] (auto state) {
    button3->set_toggled(toggled_check_box3->isChecked());
  });
  m_layout->addWidget(toggled_check_box3, 8, 2);
  m_pressed_label = new QLabel(tr("Button Pressed"), this);
  m_pressed_label->hide();
  m_pressed_timer = new QTimer(this);
  m_pressed_timer->setInterval(1000);
  m_pressed_timer->setSingleShot(true);
  connect(m_pressed_timer, &QTimer::timeout, [=] {
    m_pressed_label->hide();
  });
}

void ToggleButtonTestWidget::on_button_pressed(int row) {
  m_layout->removeWidget(m_pressed_label);
  m_layout->addWidget(m_pressed_label, row, 1);
  m_pressed_label->show();
  m_pressed_timer->start();
}
