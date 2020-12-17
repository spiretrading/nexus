#include "Spire/UiViewer/TextInputTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

using namespace Spire;

namespace {
  auto WIDGET_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }
}

TextInputTestWidget::TextInputTestWidget(QWidget* parent)
    : QWidget(parent),
      m_text_input(nullptr) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  m_text_label = new QLabel(this);
  m_text_label->setFixedSize(WIDGET_SIZE());
  m_layout->addWidget(m_text_label, 0, 1);
  m_layout->addWidget(create_parameters_label(this), 1, 0, 1, 2);
  m_set_text_input = new TextInputWidget(this);
  m_set_text_input->setFixedSize(WIDGET_SIZE());
  m_layout->addWidget(m_set_text_input, 2, 0);
  m_disable_check_box = make_check_box(tr("Disable"), this);
  m_layout->addWidget(m_disable_check_box, 2, 1);
  connect(m_disable_check_box, &CheckBox::stateChanged, [=] (auto state) {
    m_text_input->setDisabled(m_disable_check_box->isChecked());
  });
  m_set_text_button = make_flat_button(tr("Set Text"), this);
  m_set_text_button->setFixedSize(WIDGET_SIZE());
  m_set_text_button->connect_clicked_signal([=] { on_set_text_button(); });
  m_layout->addWidget(m_set_text_button, 3, 0);
  m_reset_button = make_flat_button(tr("Reset"), this);
  m_reset_button->setFixedSize(WIDGET_SIZE());
  m_reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(m_reset_button, 3, 1);
  on_reset_button();
}

void TextInputTestWidget::on_reset_button() {
  delete_later(m_text_input);
  m_text_input = new TextInputWidget(m_set_text_input->text(), this);
  m_text_input->setFixedSize(WIDGET_SIZE());
  connect(m_text_input, &TextInputWidget::editingFinished, [=] {
    m_text_label->setText(m_text_input->text());
  });
  m_disable_check_box->setChecked(false);
  m_layout->addWidget(m_text_input, 0, 0);
  setTabOrder(m_text_input, m_set_text_input);
  setTabOrder(m_set_text_input, m_disable_check_box);
  setTabOrder(m_disable_check_box, m_set_text_button);
  setTabOrder(m_set_text_button, m_reset_button);
}

void TextInputTestWidget::on_set_text_button() {
  m_text_input->setText(m_set_text_input->text());
}
