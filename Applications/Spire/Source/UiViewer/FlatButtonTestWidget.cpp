#include "Spire/UiViewer/FlatButtonTestWidget.hpp"
#include <QFont>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Spire;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }
}

FlatButtonTestWidget::FlatButtonTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container_widget = new QWidget(this);
  m_layout = new QGridLayout(container_widget);
  m_status_label = new QLabel(this);
  m_status_label->setFocusPolicy(Qt::NoFocus);
  m_layout->addWidget(m_status_label, 0, 1);
  set_button(make_flat_button(tr("FlatButton"), this));
  m_label_input = new TextInputWidget(this);
  m_label_input->setFixedSize(BUTTON_SIZE());
  m_layout->addWidget(m_label_input, 1, 0);
  connect(m_label_input, &TextInputWidget::editingFinished, [=] {
    on_create_button();
  });
  auto create_button = make_flat_button("Create Button", this);
  create_button->setFixedSize(BUTTON_SIZE());
  m_layout->addWidget(create_button, 1, 1);
  create_button->connect_clicked_signal([=] { on_create_button(); });
  m_disable_check_box = make_check_box(tr("Disable"), this);
  m_disable_check_box->setChecked(false);
  m_layout->addWidget(m_disable_check_box, 2, 1);
  connect(m_disable_check_box, &QCheckBox::stateChanged, [=] (auto state) {
    m_button->setDisabled(m_disable_check_box->isChecked());
  });
  m_pressed_timer = new QTimer(this);
  m_pressed_timer->setInterval(1000);
  m_pressed_timer->setSingleShot(true);
  connect(m_pressed_timer, &QTimer::timeout, [=] {
    m_status_label->setText("");
  });
}

void FlatButtonTestWidget::set_button(FlatButton* button) {
  m_button = button;
  m_button->setFixedSize(BUTTON_SIZE());
  m_button->connect_clicked_signal([=] {
    m_status_label->setText(tr("Button pressed."));
    m_pressed_timer->start();
  });
  m_layout->addWidget(m_button, 0, 0);
}

void FlatButtonTestWidget::on_create_button() {
  m_disable_check_box->setChecked(false);
  delete_later(m_button);
  set_button(make_flat_button(m_label_input->text(), this));
}
