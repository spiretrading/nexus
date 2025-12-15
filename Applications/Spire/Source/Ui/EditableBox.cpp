#include "Spire/Ui/EditableBox.hpp"
#include <QApplication>
#include <QKeyEvent>
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

bool EditableBox::default_edit_trigger(const QKeySequence& key) {
  if(key.count() != 1) {
    return false;
  }
  auto combination = key[0];
  auto modifier = combination & Qt::KeyboardModifierMask;
  auto key_value = combination - modifier;
  return (modifier == Qt::NoModifier || modifier == Qt::ShiftModifier ||
    modifier == Qt::KeypadModifier) &&
    (key_value >= Qt::Key_A && key_value <= Qt::Key_Z ||
      key_value >= Qt::Key_0 && key_value <= Qt::Key_9 ||
      key_value == Qt::Key_Underscore);
}

EditableBox::EditableBox(
  AnyInputBox& input_box, DefaultValue make_default_value, QWidget* parent)
  : EditableBox(input_box, std::move(make_default_value), default_edit_trigger,
      parent) {}

EditableBox::EditableBox(
    AnyInputBox& input_box, DefaultValue make_default_value,
    EditTrigger trigger, QWidget* parent)
    : QWidget(parent),
      m_input_box(&input_box),
      m_make_default_value(std::move(make_default_value)),
      m_edit_trigger(std::move(trigger)),
      m_focus_observer(*this),
      m_mouse_observer(*m_input_box),
      m_focus_proxy(nullptr),
      m_is_submit_connected(false) {
  setFocusProxy(m_input_box);
  enclose(*this, *m_input_box);
  proxy_style(*this, *m_input_box);
  setFocusPolicy(Qt::StrongFocus);
  m_focus_observer.connect_state_signal(
    std::bind_front(&EditableBox::on_focus, this));
  m_mouse_observer.connect_filtered_mouse_signal(
    std::bind_front(&EditableBox::on_click, this));
  m_input_box->set_read_only(true);
  match(*this, ReadOnly());
}

const AnyInputBox& EditableBox::get_input_box() const {
  return *m_input_box;
}

AnyInputBox& EditableBox::get_input_box() {
  return *m_input_box;
}

bool EditableBox::is_read_only() const {
  return m_input_box->is_read_only();
}

void EditableBox::set_read_only(bool read_only) {
  if(read_only == is_read_only()) {
    return;
  }
  m_input_box->set_read_only(read_only);
  if(read_only) {
    match(*this, ReadOnly());
    m_input_box->clearFocus();
  } else {
    unmatch(*this, ReadOnly());
    if(!m_is_submit_connected) {
      m_is_submit_connected = true;
      m_input_box->connect_submit_signal(
        std::bind_front(&EditableBox::on_submit, this));
    }
    m_input_box->get_highlight()->set(Highlight(-1));
    m_input_box->setFocus();
  }
  m_read_only_signal(read_only);
}

connection EditableBox::connect_read_only_signal(
    const ReadOnlySignal::slot_type& slot) const {
  return m_read_only_signal.connect(slot);
}

void EditableBox::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers() & Qt::NoModifier &&
      (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
    return;
  } else if(event->key() == Qt::Key_Escape) {
    set_read_only(true);
  } else if(event->key() == Qt::Key_Space && is_input_box_enabled()) {
    set_read_only(false);
  } else if(event->key() == Qt::Key_Backspace ||
      event->key() == Qt::Key_Delete) {
    m_input_box->get_current()->set(m_make_default_value());
  } else {
    if(!is_input_box_enabled() || !is_read_only()) {
      QWidget::keyPressEvent(event);
      return;
    }
    if(m_edit_trigger(QKeySequence(event->key() | event->modifiers()))) {
      set_read_only(false);
      select_all_text();
      if(auto focus_proxy = find_focus_proxy(*m_input_box)) {
        QCoreApplication::sendEvent(focus_proxy, event);
      } else {
        QWidget::keyPressEvent(event);
      }
    } else {
      QWidget::keyPressEvent(event);
    }
  }
}

bool EditableBox::focusNextPrevChild(bool next) {
  if(isHidden()) {
    return true;
  }
  return QWidget::focusNextPrevChild(next);
}

bool EditableBox::is_input_box_enabled() const {
  auto proxy = find_focus_proxy(*m_input_box);
  return proxy && proxy->isEnabled();
}

void EditableBox::select_all_text() {
  m_input_box->get_highlight()->set(Highlight(0, -1));
}

void EditableBox::on_focus(FocusObserver::State state) {
  if(isHidden() || m_input_box->isHidden()) {
    m_focus_time = none;
    return;
  }
  if(state == FocusObserver::State::NONE) {
    m_focus_time = none;
    set_read_only(true);
  } else {
    m_focus_time = std::chrono::steady_clock::now();
  }
}

bool EditableBox::on_click(QWidget& target, QMouseEvent& event) {
  if(!is_input_box_enabled() || !is_read_only()) {
    return false;
  }
  if(event.type() == QEvent::MouseButtonDblClick &&
      event.button() == Qt::MouseButton::LeftButton) {
    set_read_only(false);
    auto press = QMouseEvent(QEvent::MouseButtonPress,
      target.mapFrom(target.window(), event.windowPos().toPoint()), event.pos(),
      event.button(), event.buttons(), event.modifiers());
    QApplication::sendEvent(&target, &press);
    return true;
  } else if(event.type() == QEvent::MouseButtonPress &&
      event.button() == Qt::MouseButton::LeftButton) {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - m_focus_time.value_or(now)).count();
    if(duration >= 50) {
      m_focus_time = none;
      auto focus_widget = QApplication::focusWidget();
      if(!focus_widget || isAncestorOf(focus_widget)) {
        set_read_only(false);
      }
    }
  }
  return false;
}

void EditableBox::on_submit(const AnyRef& submission) {
  set_read_only(true);
}
