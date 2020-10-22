#include "Spire/UiViewer/SecurityInputTestWidget.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }
}

SecurityInputTestWidget::SecurityInputTestWidget(QWidget* parent)
    : QWidget(parent),
      m_security_input(nullptr) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  m_status_label = new QLabel(this);
  m_status_label->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_status_label, 0, 1, Qt::AlignRight);
  auto initial_text_label = new QLabel(tr("Initial Text"), this);
  m_layout->addWidget(initial_text_label, 1, 0);
  m_initial_text_input = new TextInputWidget(this);
  m_initial_text_input->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_initial_text_input, 1, 1);
  m_icon_check_box = make_check_box(tr("Show Icon"), this);
  m_layout->addWidget(m_icon_check_box, 2, 1);
  auto reset_button = make_flat_button(tr("Reset"), this);
  reset_button->setFixedHeight(scale_height(26));
  reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(reset_button, 3, 0, 1, 2);
  on_reset_button();
}

void SecurityInputTestWidget::on_reset_button() {
  delete_later(m_security_input);
  m_security_input = new SecurityInputLineEdit(m_initial_text_input->text(),
    Ref(get_local_security_input_test_model()), m_icon_check_box->isChecked(),
    this);
  m_security_input->setFixedSize(CONTROL_SIZE());
  connect(m_security_input, &TextInputWidget::editingFinished,
    [=] {
      m_status_label->setText(m_item_delegate.displayText(
        QVariant::fromValue(m_security_input->get_security())));
      m_security_input->clear();
    });
  m_status_label->setText("");
  m_security_input->setFocus();
  m_layout->addWidget(m_security_input, 0, 0);
}
