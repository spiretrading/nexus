#include "Spire/Ui/EditableBox.hpp"
#include <QKeyEvent>

using namespace Spire;

EditableBox::EditableBox(AnyInputBox& input_box,
    QWidget* parent)
    : QWidget(parent),
      m_input_box(&input_box),
      m_focus_observer(*this) {
  enclose(*this, *m_input_box);
  setFocusProxy(m_input_box);
  m_focus_observer.connect_state_signal(
    std::bind_front(&EditableBox::on_focus, this));
  m_input_box->set_read_only(true);
  //m_input_box->installEventFilter(this);
}

const AnyInputBox& EditableBox::get_input_box() const {
  return *m_input_box;
}

AnyInputBox& EditableBox::get_input_box() {
  return *m_input_box;
}

//bool EditableBox::eventFilter(QObject* watched, QEvent* event) {
//  if(watched == m_input_box && event->type() == QEvent::KeyPress) {
//    auto& key_event = static_cast<QKeyEvent&>(*event);
//    if(key_event.key() == Qt::Key_Escape) {
//      m_input_box->set_read_only(true);
//    }
//  }
//  return QWidget::eventFilter(watched, event);
//}

void EditableBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    m_input_box->set_read_only(true);
    clearFocus();
  }
  QWidget::keyPressEvent(event);
}

void EditableBox::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    m_input_box->set_read_only(true);
  } else {
    m_input_box->set_read_only(false);
  }
}
