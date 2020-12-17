#include "Spire/UiViewer/FlatButtonTestWidget.hpp"
#include <QFont>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

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
  m_layout->addWidget(make_parameters_label(this), 1, 0, 1, 2);
  m_label_input = new TextInputWidget(this);
  m_label_input->setFixedSize(BUTTON_SIZE());
  m_layout->addWidget(m_label_input, 2, 0);
  connect(m_label_input, &TextInputWidget::editingFinished, [=] {
    on_create_button();
  });
  m_create_button = make_flat_button("Create Button", this);
  m_create_button->setFixedSize(BUTTON_SIZE());
  m_layout->addWidget(m_create_button, 2, 1);
  m_create_button->connect_clicked_signal([=] { on_create_button(); });
  m_disable_check_box = make_check_box(tr("Disable"), this);
  m_disable_check_box->setChecked(false);
  m_layout->addWidget(m_disable_check_box, 3, 1);
  connect(m_disable_check_box, &QCheckBox::stateChanged, [=] (auto state) {
    m_button->setDisabled(m_disable_check_box->isChecked());
  });
  m_pressed_timer = new QTimer(this);
  m_pressed_timer->setInterval(1000);
  m_pressed_timer->setSingleShot(true);
  connect(m_pressed_timer, &QTimer::timeout, [=] {
    m_status_label->setText("");
  });
  set_button(make_flat_button(tr("FlatButton"), this));
}

void FlatButtonTestWidget::set_button(FlatButton* button) {
  m_button = button;
  m_button->setFixedSize(BUTTON_SIZE());
  m_button->connect_clicked_signal([=] {
    m_status_label->setText(tr("Button pressed."));
    m_pressed_timer->start();
  });
  m_layout->addWidget(m_button, 0, 0);
  setTabOrder(m_button, m_label_input);
  setTabOrder(m_label_input, m_create_button);
  setTabOrder(m_create_button, m_disable_check_box);
}

void FlatButtonTestWidget::on_create_button() {
  m_disable_check_box->setChecked(false);
  delete_later(m_button);
  set_button(make_flat_button(m_label_input->text(), this));
}
