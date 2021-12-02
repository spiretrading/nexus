#include "Spire/Ui/FocusObserver.hpp"
#include <QApplication>
#include <QFocusEvent>
#include "Spire/Spire/Utility.hpp"

using namespace boost::signals2;
using namespace Spire;

struct FocusObserver::FocusEventFilter : QObject {
  mutable StateSignal m_state_signal;
  const QWidget* m_widget;
  State m_state;
  State m_old_state;
  Qt::FocusReason m_focus_reason;

  FocusEventFilter(const QWidget& widget)
      : m_widget(&widget),
        m_focus_reason(Qt::MouseFocusReason),
        m_state(find_focus_state(*m_widget)) {
    m_old_state = m_state;
    qApp->installEventFilter(this);
    connect(qApp, &QApplication::focusChanged, this,
      &FocusEventFilter::on_focus_changed);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusIn && watched->isWidgetType() &&
        is_ancestor(m_widget, static_cast<QWidget*>(watched))) {
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
    auto state = m_state;
    if(m_widget == now) {
      m_state = State::FOCUS;
      switch(m_focus_reason) {
        case Qt::TabFocusReason:
        case Qt::BacktabFocusReason:
        case Qt::ShortcutFocusReason:
          m_state = State::FOCUS_VISIBLE;
          widget_focus_visible = {now, true};
          break;
        case Qt::ActiveWindowFocusReason:
        case Qt::PopupFocusReason:
          if(m_old_state == State::FOCUS_VISIBLE) {
            m_state = State::FOCUS_VISIBLE;
            widget_focus_visible = {now, true};
          }
          break;
        case Qt::OtherFocusReason:
          if(previous_widget_focus_visible.first == old &&
              previous_widget_focus_visible.second) {
            m_state = State::FOCUS_VISIBLE;
            widget_focus_visible = {now, true};
          }
          break;
        default:
          widget_focus_visible = {now, false};
          break;
        }
    } else if(is_ancestor(m_widget, now)) {
      m_state = State::FOCUS_IN;
    } else {
      m_state = State::NONE;
    }
    if(state != m_state) {
      m_old_state = state;
      m_state_signal(m_state);
    }
  }

  connection connect_state_signal(const StateSignal::slot_type& slot) const {
    return m_state_signal.connect(slot);
  }
};

FocusObserver::FocusObserver(const QWidget& widget) {
  static auto filters =
    std::unordered_map<const QWidget*, std::weak_ptr<FocusEventFilter>>();
  auto filter = filters.find(&widget);
  if(filter != filters.end()) {
    m_filter = filter->second.lock();
  } else {
    m_filter = std::shared_ptr<FocusEventFilter>(
      new FocusEventFilter(widget), [] (auto* p) {
        if(!p) {
          return;
        }
        if(p->m_widget) {
          filters.erase(p->m_widget);
        }
        p->deleteLater();
      });
    QObject::connect(&widget, &QObject::destroyed, [&widget] (auto) {
      auto filter = filters.find(&widget);
      if(filter != filters.end()) {
        filter->second.lock()->m_widget = nullptr;
        filters.erase(filter);
      }
    });
    filters.emplace(&widget, m_filter);
  }
  m_filter_connection = m_filter->m_state_signal.connect(m_state_signal);
}

FocusObserver::State FocusObserver::get_state() const {
  return m_filter->m_state;
}

connection FocusObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_state_signal.connect(slot);
}

FocusObserver::State Spire::find_focus_state(const QWidget& widget) {
  if(widget.hasFocus()) {
    return FocusObserver::State::FOCUS;
  } else if(is_ancestor(&widget, QApplication::focusWidget())) {
    return FocusObserver::State::FOCUS_IN;
  }
  return FocusObserver::State::NONE;
}

bool Spire::is_set(FocusObserver::State left, FocusObserver::State right) {
  return static_cast<FocusObserver::State>(
    static_cast<std::underlying_type_t<FocusObserver::State>>(left) &
    static_cast<std::underlying_type_t<FocusObserver::State>>(right)) !=
    FocusObserver::State::NONE;
}
