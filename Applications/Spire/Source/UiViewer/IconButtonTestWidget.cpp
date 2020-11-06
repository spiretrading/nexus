#include "Spire/UiViewer/IconButtonTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/IconButton.hpp"

using namespace Spire;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(26, 26);
    return size;
  }

  auto IMAGE_SIZE() {
    static auto rect = QSize(scale(16, 16));
    return rect;
  }
}

IconButtonTestWidget::IconButtonTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  auto label1 = new QLabel(tr("Default Image"), this);
  m_layout->addWidget(label1, 0, 0, 1, 3);
  auto button1 = new IconButton(
    imageFromSvg(":/Icons/time-sale-black.svg", IMAGE_SIZE()), this);
  button1->setToolTip(tr("Tooltip"));
  button1->setFixedSize(BUTTON_SIZE());
  button1->connect_clicked_signal([=] { on_button_pressed(1); });
  m_layout->addWidget(button1, 1, 0);
  auto disable_check_box1 = make_check_box(tr("Disable"), this);
  connect(disable_check_box1, &CheckBox::stateChanged, [=] (auto state) {
    button1->setDisabled(disable_check_box1->isChecked());
  });
  m_layout->addWidget(disable_check_box1, 1, 2);
  auto label2 = new QLabel(tr("Default and Hover Images"), this);
  m_layout->addWidget(label2, 2, 0, 1, 3);
  auto button2 = new IconButton(
    imageFromSvg(":/Icons/time-sale-black.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/time-sale-purple.svg", IMAGE_SIZE()), this);
  button2->setToolTip(tr("Tooltip"));
  button2->setFixedSize(BUTTON_SIZE());
  button2->connect_clicked_signal([=] { on_button_pressed(3); });
  m_layout->addWidget(button2, 3, 0);
  auto disable_check_box2 = make_check_box(tr("Disable"), this);
  connect(disable_check_box2, &CheckBox::stateChanged, [=] (auto state) {
    button2->setDisabled(disable_check_box2->isChecked());
  });
  m_layout->addWidget(disable_check_box2, 3, 2);
  auto label3 = new QLabel(
    tr("Default, Hover, and Window Deac. Images"), this);
  m_layout->addWidget(label3, 4, 0, 1, 3);
  auto button3 = new IconButton(
    imageFromSvg(":/Icons/time-sale-black.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/time-sale-purple.svg", IMAGE_SIZE()),
    imageFromSvg(":/Icons/time-sale-grey.svg", IMAGE_SIZE()), this);
  button3->setToolTip(tr("Tooltip"));
  button3->setFixedSize(BUTTON_SIZE());
  button3->connect_clicked_signal([=] { on_button_pressed(5); });
  m_layout->addWidget(button3, 5, 0);
  auto disable_check_box3 = make_check_box(tr("Disable"), this);
  connect(disable_check_box3, &CheckBox::stateChanged, [=] (auto state) {
    button3->setDisabled(disable_check_box3->isChecked());
  });
  m_layout->addWidget(disable_check_box3, 5, 2);
  m_pressed_label = new QLabel(tr("Button Pressed"), this);
  m_pressed_label->hide();
  m_pressed_timer = new QTimer(this);
  m_pressed_timer->setInterval(1000);
  m_pressed_timer->setSingleShot(true);
  connect(m_pressed_timer, &QTimer::timeout, [=] {
    m_pressed_label->hide();
  });
}

void IconButtonTestWidget::on_button_pressed(int row) {
  m_layout->removeWidget(m_pressed_label);
  m_layout->addWidget(m_pressed_label, row, 1);
  m_pressed_label->show();
  m_pressed_timer->start();
}
