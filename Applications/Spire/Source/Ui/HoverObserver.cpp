#include "Spire/Ui/HoverObserver.hpp"
#include <unordered_set>
#include <QApplication>
#include <qt_windows.h>

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto get_ancestors(QWidget* widget) {
    auto ancestors = std::vector<QWidget*>();
    while(widget != nullptr) {
      ancestors.push_back(widget);
      widget = widget->parentWidget();
    }
    return ancestors;
  }
}

std::unordered_map<const QWidget*, HoverObserver::Entry>
  HoverObserver::m_entries;
QTimer HoverObserver::m_poll_timer = QTimer();
QWidget* HoverObserver::m_current = nullptr;
Qt::MouseButtons HoverObserver::m_buttons = Qt::NoButton;

HoverObserver::HoverObserver(const QWidget& widget)
    : m_widget(&widget) {
  if(m_poll_timer.interval() == 0) {
    setup_timer();
  }
  if(!m_entries.contains(&widget)) {
    m_entries.insert(std::pair(&widget, Entry{State::NONE}));
    widget.connect(&widget, &QObject::destroyed, [=, &widget] {
      m_entries.erase(&widget);
    });
  }
}

HoverObserver::State HoverObserver::get_state() const {
  return m_entries.at(m_widget).m_state;
}

connection HoverObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_entries.at(m_widget).m_state_signal.connect(slot);
}

void HoverObserver::setup_timer() {
  m_poll_timer.setInterval(50);
  QObject::connect(
    &m_poll_timer, &QTimer::timeout, [=] { on_poll_timeout(); });
  m_poll_timer.start();
}

void HoverObserver::on_poll_timeout() {
  auto previous = m_current;
  m_current = qApp->widgetAt(QCursor::pos());
  auto previous_buttons = m_buttons;
  m_buttons = qApp->mouseButtons();
  if(m_current) {
    m_current->connect(m_current, &QObject::destroyed, [=] {
      m_current = nullptr;
    });
  }
  if(m_current != previous || m_buttons != previous_buttons) {
    auto previous_parent = previous;
    auto updated_widgets = get_ancestors(previous_parent);
    auto current_parent = m_current;
    auto current_widgets = get_ancestors(current_parent);
    updated_widgets.insert(
      updated_widgets.end(), current_widgets.begin(), current_widgets.end());
    auto last = std::unique(updated_widgets.begin(), updated_widgets.end());
    for(auto i = updated_widgets.begin(); i != last; ++i) {
      if(m_entries.contains(*i)) {
        auto& entry = m_entries.at(*i);
        if(m_buttons == Qt::NoButton && *i == m_current) {
          if(entry.m_state != State::MOUSE_OVER) {
            entry.m_state = State::MOUSE_OVER;
            entry.m_state_signal(entry.m_state);
          }
        } else if(m_buttons == Qt::NoButton && (*i)->isAncestorOf(m_current)) {
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
