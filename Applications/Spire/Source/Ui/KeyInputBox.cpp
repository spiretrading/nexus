#include "Spire/Ui/KeyInputBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  std::vector<Qt::Key> split(const QKeySequence& sequence) {
    auto keys = std::vector<Qt::Key>();
    for(auto& key: sequence.toString().split('+')) {
      if(key == "Shift") {
        keys.push_back(Qt::Key_Shift);
      } else if(key == "Meta") {
        keys.push_back(Qt::Key_Meta);
      } else if(key == "Ctrl") {
        keys.push_back(Qt::Key_Control);
      } else if(key == "Alt") {
        keys.push_back(Qt::Key_Alt);
      } else {
        auto name = QKeySequence(key);
        if(name.count() != 0) {
          keys.push_back(static_cast<Qt::Key>(name[0]));
        }
      }
    }
    return keys;
  }
}

KeyInputBox::KeyInputBox(
    std::shared_ptr<KeySequenceValueModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_status(Status::UNINITIALIZED) {
  setFocusPolicy(Qt::StrongFocus);
  m_body = new QWidget();
  auto layout = new QHBoxLayout();
  layout->setContentsMargins({});
  layout->addWidget(make_input_box(m_body, this));
  setLayout(layout);
  auto body_layout = new QHBoxLayout();
  body_layout->setContentsMargins({});
  m_body->setLayout(body_layout);
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
  auto& layout = *m_body->layout();
  while(auto item = layout.takeAt(0)) {
    item->widget()->deleteLater();
    delete item;
  }
  if(m_status == Status::NONE) {
    layout.setSpacing(scale_width(4));
    for(auto key : split(m_current->get_current())) {
      layout.addWidget(new KeyTag(make_constant_value_model(key)));
    }
  } else if(m_status == Status::PROMPT) {
    layout.setSpacing(0);
    auto prompt = new TextBox(tr("Enter Keys"));
    prompt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    prompt->set_read_only(true);
    prompt->setDisabled(true);
    layout.addWidget(prompt);
  }
}

void KeyInputBox::on_current(const QKeySequence& current) {
  transition_status();
}
