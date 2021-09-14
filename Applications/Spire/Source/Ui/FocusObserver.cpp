#include "Spire/Ui/FocusObserver.hpp"
#include <QApplication>
#include <QFocusEvent>

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
        m_focus_reason(Qt::MouseFocusReason) {
    if(m_widget->hasFocus()) {
      m_state = State::FOCUS;
    } else if(m_widget->isAncestorOf(QApplication::focusWidget())) {
      m_state = State::FOCUS_IN;
    } else {
      m_state = State::NONE;
    }
    m_old_state = m_state;
    qApp->installEventFilter(this);
    connect(qApp,
      &QApplication::focusChanged, this, &FocusEventFilter::on_focus_changed);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusIn && watched->isWidgetType() &&
        m_widget->isAncestorOf(static_cast<QWidget*>(watched))) {
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
          widget_focus_visible = std::pair(now, true);
          break;
        case Qt::ActiveWindowFocusReason:
        case Qt::PopupFocusReason:
          if(m_old_state == State::FOCUS_VISIBLE) {
            m_state = State::FOCUS_VISIBLE;
            widget_focus_visible = std::pair(now, true);
          }
          break;
        case Qt::OtherFocusReason:
          if(previous_widget_focus_visible.first == old &&
              previous_widget_focus_visible.second) {
            m_state = State::FOCUS_VISIBLE;
            widget_focus_visible = std::pair(now, true);
          }
          break;
        default:
          widget_focus_visible = std::pair(now, false);
          break;
        }
    } else if(m_widget->isAncestorOf(now)) {
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
  static auto widget_workers = std::unordered_map<const QWidget*,
    std::weak_ptr<FocusEventFilter>>();
  auto worker = widget_workers.find(&widget);
  if(worker != widget_workers.end()) {
    m_worker = worker->second.lock();
  } else {
    m_worker = std::shared_ptr<FocusEventFilter>(
      new FocusEventFilter(widget), [] (auto* p) {
        if(!p) {
          return;
        }
        if(p->m_widget) {
          widget_workers.erase(p->m_widget);
        }
        p->deleteLater();
      });
    QObject::connect(&widget, &QObject::destroyed, [&widget] (auto) {
      auto worker = widget_workers.find(&widget);
      if(worker != widget_workers.end()) {
        worker->second.lock()->m_widget = nullptr;
        widget_workers.erase(worker);
      }
    });
    widget_workers.emplace(&widget, m_worker);
  }
}

FocusObserver::State FocusObserver::get_state() const {
  return m_worker->m_state;
}

connection FocusObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_worker->connect_state_signal(slot);
}
