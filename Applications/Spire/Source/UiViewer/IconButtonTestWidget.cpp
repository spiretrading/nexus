#include "Spire/UiViewer/IconButtonTestWidget.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/IconButton.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

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
  auto layout = new QGridLayout(container);
  auto button = new IconButton(
    imageFromSvg(":/Icons/demo.svg", BUTTON_SIZE()), this);
  button->setToolTip(tr("Tooltip"));
  button->setFixedSize(BUTTON_SIZE());
  layout->addWidget(button, 0, 0);
  auto pressed_label = new QLabel(this);
  pressed_label->setFixedWidth(scale_width(80));
  layout->addWidget(pressed_label, 0, 1);
  auto pressed_timer = new QTimer(this);
  pressed_timer->setInterval(1000);
  pressed_timer->setSingleShot(true);
  connect(pressed_timer, &QTimer::timeout, [=] {
    pressed_label->setText("");
  });
  button->connect_clicked_signal([=] {
    pressed_label->setText(tr("Button Pressed"));
    pressed_timer->start();
  });
  layout->addWidget(make_parameters_label(this), 1, 0, 1, 2);
  auto disable_check_box = make_check_box(tr("Disable"), this);
  connect(disable_check_box, &CheckBox::stateChanged, [=] (auto state) {
    button->setDisabled(disable_check_box->isChecked());
  });
  layout->addWidget(disable_check_box, 2, 0);
}
