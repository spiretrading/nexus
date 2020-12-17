#include "Spire/UiViewer/CheckBoxTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }
}

CheckBoxTestWidget::CheckBoxTestWidget(QWidget* parent)
    : QWidget(parent),
      m_check_box(nullptr) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  m_status_label = new QLabel(this);
  m_layout->addWidget(m_status_label, 0, 1);
  m_layout->addWidget(create_parameters_label(this), 1, 0, 1, 2);
  m_label_input = new TextInputWidget(this);
  m_label_input->setPlaceholderText(tr("Label"));
  m_label_input->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_label_input, 2, 0);
  m_reset_button = make_flat_button("Reset", this);
  m_reset_button->setFixedSize(CONTROL_SIZE());
  m_reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(m_reset_button, 2, 1);
  m_label_input->setText(tr("CheckBox"));
  on_reset_button();
  m_label_input->setText("");
}

void CheckBoxTestWidget::on_reset_button() {
  delete_later(m_check_box);
  m_status_label->setText("");
  m_check_box = make_check_box(m_label_input->text(), this);
  connect(m_check_box, &CheckBox::stateChanged, [=] (auto state) {
    if(m_check_box->isChecked()) {
      m_status_label->setText(tr("Checked"));
    } else {
      m_status_label->setText(tr("Unchecked"));
    }
  });
  m_layout->addWidget(m_check_box, 0, 0);
  setTabOrder(m_check_box, m_label_input);
  setTabOrder(m_label_input, m_reset_button);
}
