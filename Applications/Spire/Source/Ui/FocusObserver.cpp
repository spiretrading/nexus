#include "Spire/Ui/FocusObserver.hpp"
#include <vector>
#include <QApplication>
#include <QFocusEvent>
#include "Spire/Spire/Utility.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  struct FocusReasonObserver : QObject {
    struct Entry {
      bool m_is_removed;
      const QWidget* m_widget;
      Qt::FocusReason* m_focus_reason;
    };
    std::vector<std::unique_ptr<Entry>> m_entries;
    std::unordered_map<const QWidget*, Entry*> m_widget_to_entry;

    FocusReasonObserver() {
      qApp->installEventFilter(this);
    }

    void add(const QWidget& widget, Qt::FocusReason& focus_reason) {
      m_entries.push_back(
        std::make_unique<Entry>(false, &widget, &focus_reason));
      m_widget_to_entry.insert(std::pair(&widget, m_entries.back().get()));
    }

    void remove(const QWidget& widget) {
      auto i = m_widget_to_entry.find(&widget);
      if(i == m_widget_to_entry.end()) {
        return;
      }
      i->second->m_is_removed = true;
      m_widget_to_entry.erase(i);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::FocusIn && watched->isWidgetType()) {
        m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
          [&] (auto& entry) {
            if(entry->m_is_removed) {
              return true;
            }
            if(is_ancestor(entry->m_widget, static_cast<QWidget*>(watched))) {
              *entry->m_focus_reason =
                static_cast<QFocusEvent*>(event)->reason();
            }
            return false;
          }), m_entries.end());
      }
      return QObject::eventFilter(watched, event);
    }
  };

  std::shared_ptr<FocusReasonObserver> get_focus_reason_observer() {
    static auto observer = std::weak_ptr<FocusReasonObserver>();
    if(auto instance = observer.lock()) {
      return instance;
    }
    auto instance = std::make_shared<FocusReasonObserver>();
    observer = instance;
    return instance;
  }
}

struct FocusObserver::FocusEventFilter : QObject {
  mutable StateSignal m_state_signal;
  const QWidget* m_widget;
  Qt::FocusReason m_focus_reason;
  State m_state;
  State m_old_state;
  std::shared_ptr<FocusReasonObserver> m_focus_reason_observer;

  FocusEventFilter(const QWidget& widget)
      : m_widget(&widget),
        m_focus_reason(Qt::MouseFocusReason),
        m_state(find_focus_state(*m_widget)),
        m_old_state(m_state),
        m_focus_reason_observer(get_focus_reason_observer()) {
    m_focus_reason_observer->add(*m_widget, m_focus_reason);
    connect(qApp, &QApplication::focusChanged, this,
      &FocusEventFilter::on_focus_changed);
  }

  ~FocusEventFilter() {
    m_focus_reason_observer->remove(*m_widget);
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

std::ostream& Spire::operator <<(std::ostream& out, FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    return out << "NONE";
  } else if(state == FocusObserver::State::FOCUS_IN) {
    return out << "FOCUS_IN";
  } else if(state == FocusObserver::State::FOCUS) {
    return out << "FOCUS";
  } else if(state == FocusObserver::State::FOCUS_VISIBLE) {
    return out << "FOCUS_VISIBLE";
  }
  return out << "?";
}
