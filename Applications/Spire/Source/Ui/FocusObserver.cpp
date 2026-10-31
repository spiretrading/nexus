#include "Spire/Ui/FocusObserver.hpp"
#include <vector>
#include <QApplication>
#include <QFocusEvent>
#include "Spire/Spire/ExtensionCache.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost;
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

  std::vector<FocusEventFilter*> m_filters;
  int m_signal_depth;

  ApplicationFocusFilter()
      : m_signal_depth(0) {
    qApp->installEventFilter(this);
    connect(qApp, &QApplication::focusChanged, this,
      &ApplicationFocusFilter::on_focus_changed);
  }

  void add(FocusEventFilter& filter);
  void remove(FocusEventFilter& filter);
  void compact();
  bool eventFilter(QObject* watched, QEvent* event) override;
  void on_focus_changed(QWidget* old, QWidget* now);
};

struct FocusObserver::FocusEventFilter {
  mutable optional<StateSignal> m_state_signal;
  const QWidget* m_widget;
  Qt::FocusReason m_focus_reason;
  State m_state;
  State m_old_state;
  int m_next_group;
  std::size_t m_index;
  std::shared_ptr<ApplicationFocusFilter> m_application_focus_filter;

  FocusEventFilter(const QWidget& widget)
      : m_widget(&widget),
        m_focus_reason(Qt::MouseFocusReason),
        m_state(find_focus_state(*m_widget)),
        m_old_state(m_state),
        m_next_group(0),
        m_index(0),
        m_application_focus_filter(ApplicationFocusFilter::get_instance()) {
    m_application_focus_filter->add(*this);
  }

  ~FocusEventFilter() {
    m_application_focus_filter->remove(*this);
  }

  connection connect_state_signal(
      int group, const StateSignal::slot_type& slot) const {
    if(!m_state_signal) {
      m_state_signal.emplace();
    }
    return m_state_signal->connect(group, slot);
  }

  void disconnect(int group) {
    if(m_state_signal) {
      m_state_signal->disconnect(group);
    }
  }
};

void FocusObserver::ApplicationFocusFilter::add(FocusEventFilter& filter) {
  filter.m_index = m_filters.size();
  m_filters.push_back(&filter);
}

void FocusObserver::ApplicationFocusFilter::remove(FocusEventFilter& filter) {
  m_filters[filter.m_index] = nullptr;
}

void FocusObserver::ApplicationFocusFilter::compact() {
  if(m_signal_depth != 0) {
    return;
  }
  auto size = std::size_t(0);
  for(auto filter : m_filters) {
    if(filter) {
      filter->m_index = size;
      m_filters[size] = filter;
      ++size;
    }
  }
  m_filters.resize(size);
}

bool FocusObserver::ApplicationFocusFilter::eventFilter(
    QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn && watched->isWidgetType()) {
    compact();
    for(auto filter : m_filters) {
      if(filter &&
          is_ancestor(filter->m_widget, static_cast<QWidget*>(watched))) {
        filter->m_focus_reason = static_cast<QFocusEvent*>(event)->reason();
      }
    }
  }
  return QObject::eventFilter(watched, event);
}

void FocusObserver::ApplicationFocusFilter::on_focus_changed(
    QWidget* old, QWidget* now) {
  static auto widget_focus_visible = std::pair<QWidget*, bool>();
  static auto previous_widget_focus_visible = widget_focus_visible;
  if(widget_focus_visible.first != now &&
      previous_widget_focus_visible != widget_focus_visible) {
    previous_widget_focus_visible = widget_focus_visible;
  }
  compact();
  auto signaling_filters = std::vector<std::size_t>();
  for(auto i = std::size_t(0); i != m_filters.size(); ++i) {
    auto filter = m_filters[i];
    if(!filter) {
      continue;
    }
    auto state = filter->m_state;
    if(filter->m_widget == now) {
      filter->m_state = State::FOCUS;
      auto reason = filter->m_focus_reason;
      if(reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason ||
          reason == Qt::ShortcutFocusReason) {
        filter->m_state = State::FOCUS_VISIBLE;
        widget_focus_visible = {now, true};
      } else if(reason == Qt::ActiveWindowFocusReason ||
          reason == Qt::PopupFocusReason) {
        if(filter->m_old_state == State::FOCUS_VISIBLE) {
          filter->m_state = State::FOCUS_VISIBLE;
          widget_focus_visible = {now, true};
        }
      } else if(reason == Qt::OtherFocusReason) {
        if(previous_widget_focus_visible.first == old &&
            previous_widget_focus_visible.second) {
          filter->m_state = State::FOCUS_VISIBLE;
          widget_focus_visible = {now, true};
        }
      } else {
        widget_focus_visible = {now, false};
      }
    } else if(is_ancestor(filter->m_widget, now)) {
      filter->m_state = State::FOCUS_IN;
    } else if(filter->m_widget == old || is_ancestor(filter->m_widget, old)) {
      filter->m_state = State::NONE;
    }
    if(state != filter->m_state) {
      filter->m_old_state = state;
      signaling_filters.push_back(i);
    }
  }
  ++m_signal_depth;
  for(auto i : signaling_filters) {
    if(auto filter = m_filters[i]) {
      if(filter->m_state_signal) {
        (*filter->m_state_signal)(filter->m_state);
      }
    }
  }
  --m_signal_depth;
}

FocusObserver::FocusObserver(const QWidget& widget)
  : m_filter(find_extension<FocusEventFilter>(widget)),
    m_group(m_filter->m_next_group++) {}

FocusObserver::~FocusObserver() {
  m_filter->disconnect(m_group);
}

FocusObserver::State FocusObserver::get_state() const {
  return m_filter->m_state;
}

connection FocusObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_filter->connect_state_signal(m_group, slot);
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

std::ostream& Spire::operator <<(
    std::ostream& out, FocusObserver::State state) {
  return out << std::format(out.getloc(), "{}", state);
}

std::format_context::iterator std::formatter<FocusObserver::State>::format(
    FocusObserver::State state, std::format_context& context) const {
  auto text = [&] {
    if(state == FocusObserver::State::NONE) {
      return "NONE";
    } else if(state == FocusObserver::State::FOCUS_IN) {
      return "FOCUS_IN";
    } else if(state == FocusObserver::State::FOCUS) {
      return "FOCUS";
    } else if(state == FocusObserver::State::FOCUS_VISIBLE) {
      return "FOCUS_VISIBLE";
    }
    return "?";
  }();
  return std::formatter<std::string_view>::format(text, context);
}
