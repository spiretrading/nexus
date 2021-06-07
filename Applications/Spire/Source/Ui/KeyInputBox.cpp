#include "Spire/Ui/KeyInputBox.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/LocalValueModel.hpp"
#include "Spire/Ui/TextBox.hpp"

#include <QApplication>

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_key_sequence(const std::vector<Qt::Key>& keys) {
    switch(keys.size()) {
      case 1:
        return QKeySequence(keys[0]);
      case 2:
        return QKeySequence(keys[0], keys[1]);
      case 3:
        return QKeySequence(keys[0], keys[1], keys[2]);
      case 4:
        return QKeySequence(keys[0], keys[1], keys[2], keys[3]);
      default:
        return QKeySequence();
    }
  }
}

KeyInputBox::KeyInputBox(QWidget* parent)
    : QWidget(parent),
      m_model(std::make_shared<LocalKeySequenceModel>(Qt::Key_Control)) {
  setFocusPolicy(Qt::StrongFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(this);
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_text_box);
  m_model->connect_current_signal([=] (const auto& sequence) {
    on_current_sequence(sequence);
  });
  on_current_sequence(m_model->get_current());
}

bool KeyInputBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    if(m_model->get_current().isEmpty()) {
      set_status(Status::PROMPT);
    }
  } else if(event->type() == QEvent::FocusOut) {
    set_status(Status::NONE);
  }
  return QWidget::eventFilter(watched, event);
}

void KeyInputBox::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      set_status(Status::PROMPT);
      break;
    case Qt::Key_Delete:
      m_model->set_current({});
      m_submission = {};
      m_submit_signal(m_submission);
      break;
    case Qt::Key_Alt:
    case Qt::Key_Control:
    case Qt::Key_Shift:
      break;
    default:
      auto sequence = [&] {
        auto keys = std::vector<Qt::Key>();
        if(event->modifiers().testFlag(Qt::AltModifier)) {
          keys.push_back(Qt::Key_Alt);
        }
        if(event->modifiers().testFlag(Qt::ControlModifier)) {
          keys.push_back(Qt::Key_Control);
        }
        if(event->modifiers().testFlag(Qt::ShiftModifier)) {
          keys.push_back(Qt::Key_Shift);
        }
        keys.push_back(Qt::Key(event->key()));
        return make_key_sequence(keys);
      }();
      m_model->set_current(sequence);
      m_submission = sequence;
      m_submit_signal(m_submission);
  }
  QWidget::keyPressEvent(event);
}

void KeyInputBox::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    set_status(Status::PROMPT);
  }
  QWidget::mousePressEvent(event);
}

const QKeySequence& KeyInputBox::get_submission() const {
  return m_submission;
}

const std::shared_ptr<KeySequenceModel>& KeyInputBox::get_model() const {
  return m_model;
}

connection KeyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void KeyInputBox::set_status(Status status) {
  //clear_layout(m_box_layout);
  //if(status == Status::PROMPT) {
  //  auto enter_keys_label = new TextBox(tr("Enter Keys"), this);
  //  enter_keys_label->setSizePolicy(QSizePolicy::Expanding,
  //    QSizePolicy::Expanding);
  //  enter_keys_label->setDisabled(true);
  //  enter_keys_label->set_read_only(true);
  //  m_box_layout->addWidget(enter_keys_label);
  //} else {
  //  auto sequence = m_model->get_current();
  //  for(auto i = 0; i < sequence.count(); ++i) {
  //    m_box_layout->addWidget(new KeyTag(Qt::Key(sequence[i]), this));
  //  }
  //}
}

void KeyInputBox::on_current_sequence(const QKeySequence& sequence) {
  if(sequence.isEmpty() && hasFocus()) {
    set_status(Status::PROMPT);
  } else {
    set_status(Status::NONE);
  }
}
