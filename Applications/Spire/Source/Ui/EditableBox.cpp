#include "Spire/Ui/EditableBox.hpp"
#include <QCoreApplication>
#include <QKeyEvent>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace{
  auto reset(AnyRef& any) {
    if(any.get_type() == typeid(QString)) {
      static auto value = QString("");
      any = AnyRef(value);
    } else if(any.get_type() == typeid(Region)) {
      static auto value = Region();
      any = AnyRef(value);
    } else if(any.get_type() == typeid(Destination)) {
      static auto value = Destination();
      any = AnyRef(value);
    } else if(any.get_type() == typeid(OrderType)) {
      static auto value = OrderType(OrderType::NONE);
      any = AnyRef(value);
    } else if(any.get_type() == typeid(Side)) {
      static auto value = Side(Side::NONE);
      any = AnyRef(value);
    } else if(any.get_type() == typeid(TimeInForce)) {
      static auto value = TimeInForce(TimeInForce::Type::NONE);
      any = AnyRef(value);
    } else if(any.get_type() == typeid(optional<Quantity>)) {
      static auto value = optional<Quantity>();
      any = AnyRef(value);
    } else if(any.get_type() == typeid(QKeySequence)) {
      static auto value = QKeySequence();
      any = AnyRef(value);
    } else {
      any = AnyRef();
    }
    return any;
  }
}

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

EditableBox::EditableBox(AnyInputBox& input_box, QWidget* parent)
  : EditableBox(input_box, default_edit_trigger, parent) {}

EditableBox::EditableBox(
    AnyInputBox& input_box, EditTrigger trigger, QWidget* parent)
    : QWidget(parent),
      m_input_box(&input_box),
      m_edit_trigger(std::move(trigger)),
      m_focus_observer(*this),
      m_focus_proxy(nullptr) {
  setFocusProxy(m_input_box);
  enclose(*this, *m_input_box);
  setFocusPolicy(Qt::StrongFocus);
  m_focus_observer.connect_state_signal(
    std::bind_front(&EditableBox::on_focus, this));
  m_input_box->set_read_only(true);
  match(*this, ReadOnly());
  proxy_style(*this, *m_input_box);
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
    m_end_edit_signal();
  } else {
    unmatch(*this, ReadOnly());
    install_focus_proxy_event_filter();
    if(auto line_edit = dynamic_cast<QLineEdit*>(m_focus_proxy)) {
      line_edit->setCursorPosition(line_edit->text().length());
    }
    m_input_box->setFocus();
    m_start_edit_signal();
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
  if(watched == m_focus_proxy &&
      event->type() == QEvent::KeyPress && is_read_only()) {
    if(static_cast<QKeyEvent*>(event)->key() == Qt::Key_Backspace) {
      event->ignore();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void EditableBox::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers() & Qt::NoModifier &&
      (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
    if(!event->isAutoRepeat()) {
      set_read_only(false);
    }
  } else if(event->key() == Qt::Key_Escape) {
    set_read_only(true);
  } else if(event->key() == Qt::Key_Backspace) {
    auto current = m_input_box->get_current()->get();
    m_input_box->get_current()->set(reset(current));
  } else {
    if(!is_read_only()) {
      return;
    }
    if(m_edit_trigger(QKeySequence(event->key() | event->modifiers()))) {
      set_read_only(false);
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
  if(isHidden() || m_input_box->isHidden()) {
    return;
  }
  set_read_only(state == FocusObserver::State::NONE);
}

void EditableBox::on_submit(const AnyRef& submission) {
  set_read_only(true);
}
