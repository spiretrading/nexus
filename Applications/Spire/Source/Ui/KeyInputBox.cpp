#include "Spire/Ui/KeyInputBox.hpp"
#include <QHBoxLayout>
#include <Spire/Ui/Box.hpp>
#include <Spire/Ui/KeyTag.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

KeyInputBox::KeyInputBox(
    std::shared_ptr<KeySequenceValueModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_status(Status::UNINITIALIZED) {
  auto layout = new QHBoxLayout();
  layout->setContentsMargins({});
  layout->addWidget(make_input_box(nullptr, this));
  setLayout(layout);
  set_status(Status::NONE);
  m_current_connection = m_current->connect_current_signal(
    [=] (const auto& current) { on_current(current); });
}

KeyInputBox::KeyInputBox(QWidget* parent)
  : KeyInputBox(std::make_shared<LocalKeySequenceValueModel>(), parent) {}

const std::shared_ptr<KeySequenceValueModel>& KeyInputBox::get_current() const {
  return m_current;
}

connection KeyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void KeyInputBox::focusInEvent(QFocusEvent* event) {
  transition_status();
}

void KeyInputBox::focusOutEvent(QFocusEvent* event) {
  set_status(Status::NONE);
}

void KeyInputBox::transition_status() {
  if(m_current->get_current().count() == 0) {
    set_status(Status::PROMPT);
  } else {
    set_status(Status::NONE);
  }
}

void KeyInputBox::set_status(Status status) {
  if(m_status == status) {
    return;
  }
  m_status = status;
}

void KeyInputBox::on_current(const QKeySequence& current) {
  transition_status();
}
