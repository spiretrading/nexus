#include "Spire/Ui/FocusObserver.hpp"
#include <QApplication>
#include <QFocusEvent>

using namespace boost::signals2;
using namespace Spire;

struct FocusObserver::FocusEventFilter : QObject {
  FocusObserver* m_observer;
  Qt::FocusReason m_focus_reason;

  FocusEventFilter(FocusObserver& observer)
      : QObject(nullptr),
        m_observer(&observer),
        m_focus_reason(Qt::MouseFocusReason) {
    qApp->installEventFilter(this);
    QObject::connect(qApp,
      &QApplication::focusChanged, this, &FocusEventFilter::on_focus_changed);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusIn && watched->isWidgetType() &&
        m_observer->m_widget->isAncestorOf(static_cast<QWidget*>(watched))) {
      m_focus_reason = static_cast<QFocusEvent*>(event)->reason();
    }
    return QObject::eventFilter(watched, event);
  }

  void on_focus_changed(QWidget* old, QWidget* now) {
    static auto widget_focus_visible = std::pair<QWidget*, bool>();
    static auto previous_widget_focus_visible = widget_focus_visible;
    if(widget_focus_visible.first != now &&
        previous_widget_focus_visible != widget_focus_visible) {
      previous_widget_focus_visible = widget_focus_visible;
    }
    auto state = m_observer->m_state;
    if(m_observer->m_widget == now) {
      m_observer->m_state = State::FOCUS | State::FOCUS_IN;
    } else if(m_observer->m_widget->isAncestorOf(now)) {
      m_observer->m_state = State::FOCUS_IN;
    } else {
      m_observer->m_state = State::NONE;
    }
    if(m_observer->m_state != State::NONE) {
      switch(m_focus_reason) {
        case Qt::TabFocusReason:
        case Qt::BacktabFocusReason:
        case Qt::ShortcutFocusReason:
          m_observer->m_state |= State::FOCUS_VISIBLE;
          widget_focus_visible = std::make_pair(now, true);
          break;
        case Qt::ActiveWindowFocusReason:
        case Qt::PopupFocusReason:
          if((m_observer->m_old_state & State::FOCUS_VISIBLE) ==
              State::FOCUS_VISIBLE) {
            m_observer->m_state |= State::FOCUS_VISIBLE;
            widget_focus_visible = std::make_pair(now, true);
          }
          break;
        case Qt::OtherFocusReason:
          if(previous_widget_focus_visible.first == old &&
              previous_widget_focus_visible.second) {
            m_observer->m_state |= State::FOCUS_VISIBLE;
            widget_focus_visible = std::make_pair(now, true);
          }
          break;
        default:
          widget_focus_visible = std::make_pair(now, false);
          break;
        }
    }
    if(state != m_observer->m_state) {
      m_observer->m_old_state = state;
      m_observer->m_state_signal(m_observer->m_state);
    }
  }
};

FocusObserver::FocusObserver(const QWidget& widget)
    : m_widget(&widget),
      m_state(State::NONE) {
  if(m_widget->hasFocus()) {
    m_state = State::FOCUS | State::FOCUS_IN;
  } else if(m_widget->isAncestorOf(QApplication::focusWidget())) {
    m_state = State::FOCUS_IN;
  }
  m_old_state = m_state;
  m_focus_event_filter = std::make_shared<FocusEventFilter>(*this);
}

FocusObserver::State FocusObserver::get_state() const {
  return m_state;
}

connection FocusObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_state_signal.connect(slot);
}

FocusObserver::State Spire::operator |(FocusObserver::State left,
    FocusObserver::State right) {
  return static_cast<FocusObserver::State>(
    static_cast<std::underlying_type_t<FocusObserver::State>>(left) |
    static_cast<std::underlying_type_t<FocusObserver::State>>(right));
}

FocusObserver::State Spire::operator &(FocusObserver::State left,
    FocusObserver::State right) {
  return static_cast<FocusObserver::State>(
    static_cast<std::underlying_type_t<FocusObserver::State>>(left) &
    static_cast<std::underlying_type_t<FocusObserver::State>>(right));
}

FocusObserver::State& Spire::operator |=(FocusObserver::State& left,
    FocusObserver::State right) {
  return left = left | right;
}
