#include "Spire/UiViewer/CheckBoxTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"

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
  m_layout->setContentsMargins({});
  m_status_label = new QLabel(this);
  m_layout->addWidget(m_status_label, 0, 1);
  m_label_input = new TextInputWidget(this);
  m_label_input->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_label_input, 1, 0);
  auto reset_button = make_flat_button("Reset", this);
  reset_button->setFixedSize(CONTROL_SIZE());
  reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(reset_button, 1, 1);
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
}
