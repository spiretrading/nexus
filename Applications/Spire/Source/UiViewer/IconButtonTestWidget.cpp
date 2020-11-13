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
}

IconButtonTestWidget::IconButtonTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  auto button = new IconButton(
    imageFromSvg(":/Icons/demo.svg", BUTTON_SIZE()), this);
  button->setToolTip(tr("Tooltip"));
  button->setFixedSize(BUTTON_SIZE());
  button->connect_clicked_signal([=] {
    m_pressed_label->setText(tr("Button Pressed"));
    m_pressed_timer->start();
  });
  m_layout->addWidget(button, 0, 0);
  auto disable_check_box = make_check_box(tr("Disable"), this);
  connect(disable_check_box, &CheckBox::stateChanged, [=] (auto state) {
    button->setDisabled(disable_check_box->isChecked());
  });
  m_layout->addWidget(disable_check_box, 0, 1);
  m_pressed_label = new QLabel(this);
  m_pressed_label->setFixedWidth(scale_width(80));
  m_layout->addWidget(m_pressed_label, 1, 0);
  m_pressed_timer = new QTimer(this);
  m_pressed_timer->setInterval(1000);
  m_pressed_timer->setSingleShot(true);
  connect(m_pressed_timer, &QTimer::timeout, [=] {
    m_pressed_label->setText("");
  });
}
