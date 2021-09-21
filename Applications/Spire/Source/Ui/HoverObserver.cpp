#include "Spire/Ui/HoverObserver.hpp"
#include <unordered_set>
#include <QApplication>
#include <qt_windows.h>

using namespace boost::signals2;
using namespace Spire;

std::unordered_map<const QWidget*, HoverObserver::Entry>
  HoverObserver::m_entries;
QTimer HoverObserver::m_poll_timer = QTimer();
QWidget* HoverObserver::m_current = nullptr;

HoverObserver::HoverObserver(const QWidget& widget)
    : m_widget(&widget) {
  if(m_poll_timer.interval() == 0) {
    m_poll_timer.setInterval(50);
    QObject::connect(
      &m_poll_timer, &QTimer::timeout, [=] { on_poll_timeout(); });
    m_poll_timer.start();
  }
  if(!m_entries.contains(&widget)) {
    // TODO: initial state
    m_entries.insert(std::pair(&widget, Entry{State::NONE}));
    // TODO: when widget is destroyed
  }
}

HoverObserver::State HoverObserver::get_state() const {
  return m_entries.at(m_widget).m_state;
}

connection HoverObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_entries.at(m_widget).m_state_signal.connect(slot);
}

void HoverObserver::on_poll_timeout() {
  auto previous = m_current;
  m_current = qApp->widgetAt(QCursor::pos());
  if(m_current != previous) {
    auto updated_widgets = std::vector<QWidget*>();
    auto previous_parent = previous;
    while(previous_parent != nullptr) {
      updated_widgets.push_back(previous_parent);
      previous_parent = previous_parent->parentWidget();
    }
    auto current_parent = m_current;
    while(current_parent != nullptr) {
      updated_widgets.push_back(current_parent);
      current_parent = current_parent->parentWidget();
    }
    auto last = std::unique(updated_widgets.begin(), updated_widgets.end());
    for(auto i = updated_widgets.begin(); i != last; ++i) {
      if(m_entries.contains(*i)) {
        auto& entry = m_entries.at(*i);
        if(*i == m_current) {
          if(entry.m_state != State::MOUSE_OVER) {
            entry.m_state = State::MOUSE_OVER;
            entry.m_state_signal(entry.m_state);
          }
        } else if((*i)->isAncestorOf(m_current)) {
          if(entry.m_state != State::MOUSE_IN) {
            entry.m_state = State::MOUSE_IN;
            entry.m_state_signal(entry.m_state);
          }
        } else if(entry.m_state != State::NONE) {
          entry.m_state = State::NONE;
          entry.m_state_signal(entry.m_state);
        }
      }
    }
  }
}

bool Spire::is_set(HoverObserver::State left, HoverObserver::State right) {
  return static_cast<HoverObserver::State>(
    static_cast<std::underlying_type_t<HoverObserver::State>>(left) &
    static_cast<std::underlying_type_t<HoverObserver::State>>(right)) !=
    HoverObserver::State::NONE;
}
