#include "Spire/Ui/FocusObserver.hpp"
#include <vector>
#include <QApplication>
#include <QFocusEvent>
#include "Spire/Spire/ExtensionCache.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost::signals2;
using namespace Spire;

struct FocusObserver::ApplicationFocusFilter : QObject {
  static std::shared_ptr<ApplicationFocusFilter> get_instance() {
    static auto observer = std::weak_ptr<ApplicationFocusFilter>();
    if(auto instance = observer.lock()) {
      return instance;
    }
    auto instance = std::make_shared<ApplicationFocusFilter>();
    observer = instance;
    return instance;
  }

  struct Entry {
    bool m_is_removed;
    FocusEventFilter* m_filter;
  };
  std::vector<std::unique_ptr<Entry>> m_entries;
  std::unordered_map<FocusEventFilter*, Entry*> m_filter_to_entry;

  ApplicationFocusFilter() {
    qApp->installEventFilter(this);
    connect(qApp, &QApplication::focusChanged, this,
      &ApplicationFocusFilter::on_focus_changed);
  }

  void add(FocusEventFilter& filter) {
    m_entries.push_back(std::make_unique<Entry>(false, &filter));
    m_filter_to_entry.insert(std::pair(&filter, m_entries.back().get()));
  }

  void remove(FocusEventFilter& filter) {
    auto i = m_filter_to_entry.find(&filter);
    if(i == m_filter_to_entry.end()) {
      return;
    }
    i->second->m_is_removed = true;
    m_filter_to_entry.erase(i);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusIn && watched->isWidgetType()) {
      m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
        [&] (auto& entry) {
          if(entry->m_is_removed) {
            return true;
          }
          if(is_ancestor(
              entry->m_filter->m_widget, static_cast<QWidget*>(watched))) {
            entry->m_filter->m_focus_reason =
              static_cast<QFocusEvent*>(event)->reason();
          }
          return false;
        }), m_entries.end());
    }
    return QObject::eventFilter(watched, event);
  }

  void on_focus_changed(QWidget* old, QWidget* now);
};

struct FocusObserver::FocusEventFilter {
  mutable StateSignal m_state_signal;
  const QWidget* m_widget;
  Qt::FocusReason m_focus_reason;
  State m_state;
  State m_old_state;
  std::shared_ptr<ApplicationFocusFilter> m_application_focus_filter;

  FocusEventFilter(const QWidget& widget)
      : m_widget(&widget),
        m_focus_reason(Qt::MouseFocusReason),
        m_state(find_focus_state(*m_widget)),
        m_old_state(m_state),
        m_application_focus_filter(ApplicationFocusFilter::get_instance()) {
    m_application_focus_filter->add(*this);
  }

  ~FocusEventFilter() {
    m_application_focus_filter->remove(*this);
  }

  connection connect_state_signal(const StateSignal::slot_type& slot) const {
    return m_state_signal.connect(slot);
  }
};

void FocusObserver::ApplicationFocusFilter::on_focus_changed(
    QWidget* old, QWidget* now) {
  static auto widget_focus_visible = std::pair<QWidget*, bool>();
  static auto previous_widget_focus_visible = widget_focus_visible;
  if(widget_focus_visible.first != now &&
      previous_widget_focus_visible != widget_focus_visible) {
    previous_widget_focus_visible = widget_focus_visible;
  }
  auto signaling_entries = std::vector<Entry*>();
  std::erase_if(m_entries, [&] (const auto& entry) {
    if(entry->m_is_removed) {
      return true;
    }
    auto state = entry->m_filter->m_state;
    if(entry->m_filter->m_widget == now) {
      entry->m_filter->m_state = State::FOCUS;
      switch(entry->m_filter->m_focus_reason) {
        case Qt::TabFocusReason:
        case Qt::BacktabFocusReason:
        case Qt::ShortcutFocusReason:
          entry->m_filter->m_state = State::FOCUS_VISIBLE;
          widget_focus_visible = {now, true};
          break;
        case Qt::ActiveWindowFocusReason:
        case Qt::PopupFocusReason:
          if(entry->m_filter->m_old_state == State::FOCUS_VISIBLE) {
            entry->m_filter->m_state = State::FOCUS_VISIBLE;
            widget_focus_visible = {now, true};
          }
          break;
        case Qt::OtherFocusReason:
          if(previous_widget_focus_visible.first == old &&
              previous_widget_focus_visible.second) {
            entry->m_filter->m_state = State::FOCUS_VISIBLE;
            widget_focus_visible = {now, true};
          }
          break;
        default:
          widget_focus_visible = {now, false};
          break;
      }
    } else if(is_ancestor(entry->m_filter->m_widget, now)) {
      entry->m_filter->m_state = State::FOCUS_IN;
    } else if(entry->m_filter->m_widget == old ||
        is_ancestor(entry->m_filter->m_widget, old)) {
      entry->m_filter->m_state = State::NONE;
    }
    if(state != entry->m_filter->m_state) {
      entry->m_filter->m_old_state = state;
      signaling_entries.push_back(entry.get());
    }
    return false;
  });
  for(auto& signaling_entry : signaling_entries) {
    signaling_entry->m_filter->m_state_signal(
      signaling_entry->m_filter->m_state);
  }
}

FocusObserver::FocusObserver(const QWidget& widget) {
  m_filter = find_extension<FocusEventFilter>(widget);
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
  if(widget.isVisible()) {
    if(widget.hasFocus()) {
      return FocusObserver::State::FOCUS;
    } else if(is_ancestor(&widget, QApplication::focusWidget())) {
      return FocusObserver::State::FOCUS_IN;
    }
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
