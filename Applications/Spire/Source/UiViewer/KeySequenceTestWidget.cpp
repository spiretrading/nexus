#include "Spire/UiViewer/KeySequenceTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Spire/TestKeySequenceValidationModel.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

namespace {
  auto INPUT_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }
}

KeySequenceTestWidget::KeySequenceTestWidget(QWidget* parent)
    : QWidget(parent),
      m_input(nullptr) {
  auto container_widget = new QWidget(this);
  m_layout = new QGridLayout(container_widget);
  m_status_label = new QLabel(this);
  m_layout->addWidget(m_status_label, 0, 1);
  m_set_input = new TextInputWidget(this);
  m_set_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(m_set_input, 1, 0);
  auto set_button = make_flat_button(tr("Set Sequence"), this);
  set_button->setFixedSize(INPUT_SIZE());
  set_button->connect_clicked_signal([=] { on_set_button(); });
  m_layout->addWidget(set_button, 1, 1);
  m_reset_input = new TextInputWidget("ctrl+f1", this);
  m_reset_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(m_reset_input, 2, 0);
  auto reset_button = make_flat_button(tr("Reset Sequence"), this);
  reset_button->setFixedSize(INPUT_SIZE());
  reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(reset_button, 2, 1);
  on_reset_button();
}

void KeySequenceTestWidget::on_reset_button() {
  if(auto sequence = QKeySequence::fromString(m_reset_input->text());
      !sequence.isEmpty()) {
    delete_later(m_input);
    m_input = new KeySequenceInputField(
      std::make_shared<TestKeySequenceValidationModel>(), this);
    m_input->setFixedSize(scale(130, 26));
    m_layout->addWidget(m_input, 0, 0);
    connect(m_input, &KeySequenceInputField::editingFinished, [=] {
      m_status_label->setText(m_input->get_key_sequence().toString());
    });
    m_input->set_key_sequence(sequence);
  }
}

void KeySequenceTestWidget::on_set_button() {
  if(auto sequence = QKeySequence::fromString(m_set_input->text());
      !sequence.isEmpty()) {
    m_input->set_key_sequence(sequence);
  }
}
