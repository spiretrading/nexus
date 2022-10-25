#include "Spire/Ui/EditableBox.hpp"
#include <QCoreApplication>
#include <QKeyEvent>

using namespace boost::signals2;
using namespace Spire;

EditableBox::EditableBox(AnyInputBox& input_box, QWidget* parent)
    : QWidget(parent),
      m_input_box(&input_box),
      m_focus_observer(*this),
      m_focus_proxy(nullptr) {
  setFocusProxy(m_input_box);
  enclose(*this, *m_input_box);
  m_focus_observer.connect_state_signal(
    std::bind_front(&EditableBox::on_focus, this));
  m_input_box->set_read_only(true);
  m_submit_connection = m_input_box->connect_submit_signal(
    std::bind_front(&EditableBox::on_submit, this));
  install_focus_proxy_event_filter();
}

const AnyInputBox& EditableBox::get_input_box() const {
  return *m_input_box;
}

AnyInputBox& EditableBox::get_input_box() {
  return *m_input_box;
}

bool EditableBox::is_editing() const {
  return !m_input_box->is_read_only();
}

void EditableBox::set_editing(bool is_editing) {
  if(is_editing) {
    m_input_box->setFocus();
  } else {
    m_input_box->clearFocus();
  }
}

connection EditableBox::connect_start_edit_signal(
    const StartEditSignal::slot_type& slot) const {
  return m_start_edit_signal.connect(slot);
}

connection EditableBox::connect_end_edit_signal(
    const EndEditSignal::slot_type& slot) const {
  return m_end_edit_signal.connect(slot);
}

bool EditableBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_focus_proxy && event->type() == QEvent::KeyPress &&
      !is_editing()) {
    if(static_cast<QKeyEvent*>(event)->key() == Qt::Key_Backspace) {
      event->ignore();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void EditableBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    set_editing(true);
  } else if(event->key() == Qt::Key_Escape) {
    set_editing(false);
  } else if(event->key() == Qt::Key_Backspace) {
    m_input_box->get_current()->set(AnyRef());
  } else {
    if(is_editing()) {
      return;
    }
    if(is_a_word(event->text())) {
      set_editing(true);
      select_all_text();
      QCoreApplication::sendEvent(m_focus_proxy, event);
    } else {
      QWidget::keyPressEvent(event);
    }
  }
}

void EditableBox::showEvent(QShowEvent* event) {
  install_focus_proxy_event_filter();
  QWidget::showEvent(event);
}

bool EditableBox::focusNextPrevChild(bool next) {
  if(isHidden()) {
    return true;
  }
  return QWidget::focusNextPrevChild(next);
}

void EditableBox::install_focus_proxy_event_filter() {
  if(auto proxy = find_focus_proxy(*m_input_box); proxy != m_focus_proxy) {
    if(m_focus_proxy) {
      m_focus_proxy->removeEventFilter(this);
    }
    m_focus_proxy = proxy;
    m_focus_proxy->installEventFilter(this);
  }
}

void EditableBox::select_all_text() {
  if(auto line_edit = dynamic_cast<QLineEdit*>(m_focus_proxy)) {
    line_edit->selectAll();
  }
}

void EditableBox::on_focus(FocusObserver::State state) {
  if(isHidden()) {
    return;
  }
  if(state == FocusObserver::State::NONE) {
    if(is_editing()) {
      m_input_box->set_read_only(true);
      m_end_edit_signal();
    }
  } else if(!is_editing()) {
    m_input_box->set_read_only(false);
    install_focus_proxy_event_filter();
    if(auto line_edit = dynamic_cast<QLineEdit*>(m_focus_proxy)) {
      line_edit->setCursorPosition(line_edit->text().length());
    }
    m_start_edit_signal();
  }
}

void EditableBox::on_submit(const AnyRef& submission) {
  set_editing(false);
}
