#include "Spire/Ui/HoverObserver.hpp"
#include <unordered_map>
#include <vector>
#include <QApplication>
#include <QChildEvent>
#include <QPointer>
#include <QTimer>
#include "Spire/Spire/ExtensionCache.hpp"
#include "Spire/Ui/GlobalPositionObserver.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  HoverObserver::State get_state(const QWidget& widget, QPoint position) {
    if(!widget.isEnabled()) {
      return HoverObserver::State::NONE;
    }
    auto cursor = QCursor::pos();
    if(QRect(position, widget.size()).contains(cursor)) {
      auto hovered_widget = qApp->widgetAt(cursor);
      if(hovered_widget == &widget) {
        return HoverObserver::State::MOUSE_IN;
      } else if(widget.isAncestorOf(hovered_widget)) {
        return HoverObserver::State::MOUSE_OVER;
      }
    }
    return HoverObserver::State::NONE;
  }
}

struct HoverObserver::EventFilter : QObject {
  struct Descendant {
    const QObject* m_parent;
    std::vector<const QObject*> m_children;

    Descendant(const QObject* parent)
      : m_parent(parent) {}
  };
  mutable StateSignal m_state_signal;
  bool m_block_signal;
  QPointer<QWidget> m_widget;
  State m_state;
  std::unique_ptr<GlobalPositionObserver> m_position_observer;
  std::unordered_map<const QObject*, Descendant> m_descendants;
  QTimer m_hover_timer;

  EventFilter(QWidget& widget)
      : m_widget(&widget),
        m_block_signal(false),
        m_state(State::NONE),
        m_hover_timer(this) {
    widget.connect(
      &widget, &QObject::destroyed, this, &EventFilter::destroy_observers);
    widget.installEventFilter(this);
    if(widget.isEnabled() && widget.isVisible()) {
      initialize_observers();
    }
    const auto HOVER_PERIOD_MS = 300;
    m_hover_timer.setInterval(HOVER_PERIOD_MS);
    connect(&m_hover_timer, &QTimer::timeout, this, &EventFilter::on_timeout);
  }

  void initialize_observers() {
    m_position_observer = std::make_unique<GlobalPositionObserver>(*m_widget);
    m_position_observer->connect_position_signal(
      std::bind_front(&EventFilter::on_position, this));
    set_state(::get_state(*m_widget, m_position_observer->get_position()));
    for(auto child : m_widget->children()) {
      if(child && child->isWidgetType()) {
        add(static_cast<QWidget&>(*child), *m_widget);
      }
    }
    connect(
      qApp, &QApplication::focusChanged, this, &EventFilter::on_focus_changed);
  }

  void destroy_observers() {
    if(m_position_observer) {
      m_position_observer = nullptr;
      if(m_widget) {
        for(auto child : m_widget->children()) {
          if(child && child->isWidgetType()) {
            remove(*child);
          }
        }
      }
      m_descendants.clear();
      disconnect(qApp, &QApplication::focusChanged, this, nullptr);
      set_state(State::NONE);
    }
  }

  GlobalPositionObserver& get_position_observer() {
    if(!m_position_observer) {
      initialize_observers();
    }
    return *m_position_observer;
  }

  void set_state(State state) {
    if(state == m_state) {
      return;
    }
    m_state = state;
    if(state != State::NONE) {
      m_hover_timer.start();
    }
    if(!m_block_signal) {
      m_state_signal(state);
    }
  }

  void update_state() {
    if(m_widget) {
      set_state(::get_state(*m_widget, get_position_observer().get_position()));
    }
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(watched != m_widget) {
      if(m_position_observer) {
        on_descendant_event(*watched, *event);
      }
    } else if(!m_widget->isEnabled() || !m_widget->isVisible()) {
      destroy_observers();
    } else if(event->type() == QEvent::Enter ||
        event->type() == QEvent::EnabledChange ||
        event->type() == QEvent::MouseMove || event->type() == QEvent::Show) {
      m_block_signal = true;
      auto position = get_position_observer().get_position();
      m_block_signal = false;
      set_state(::get_state(*m_widget, position));
    } else if(event->type() == QEvent::Leave) {
      set_state(State::NONE);
    } else if(event->type() == QEvent::ChildAdded ||
        event->type() == QEvent::ChildPolished) {
      auto& child = *static_cast<QChildEvent&>(*event).child();
      if(child.isWidgetType()) {
        get_position_observer();
        if(add(static_cast<QWidget&>(child), *m_widget)) {
          update_state();
        }
      }
    } else if(event->type() == QEvent::ChildRemoved) {
      auto& child = *static_cast<QChildEvent&>(*event).child();
      if(m_position_observer && remove(child)) {
        update_state();
      }
    }
    return QObject::eventFilter(watched, event);
  }

  void on_descendant_event(const QObject& watched, QEvent& event) {
    if(event.type() == QEvent::Enter || event.type() == QEvent::Leave ||
        event.type() == QEvent::MouseMove || event.type() == QEvent::Show ||
        event.type() == QEvent::Hide || event.type() == QEvent::EnabledChange ||
        event.type() == QEvent::Move) {
      update_state();
    } else if(event.type() == QEvent::ChildAdded ||
        event.type() == QEvent::ChildPolished) {
      auto& child = *static_cast<QChildEvent&>(event).child();
      if(child.isWidgetType() && add(static_cast<QWidget&>(child), watched)) {
        update_state();
      }
    } else if(event.type() == QEvent::ChildRemoved &&
        remove(*static_cast<QChildEvent&>(event).child())) {
      update_state();
    }
  }

  bool add(QWidget& child, const QObject& parent) {
    if(!m_descendants.try_emplace(&child, &parent).second) {
      return false;
    }
    auto i = m_descendants.find(&parent);
    if(i != m_descendants.end()) {
      i->second.m_children.push_back(&child);
    }
    child.installEventFilter(this);
    for(auto descendant : child.children()) {
      if(descendant && descendant->isWidgetType()) {
        add(static_cast<QWidget&>(*descendant), child);
      }
    }
    return true;
  }

  bool remove(QObject& child) {
    auto i = m_descendants.find(&child);
    if(i == m_descendants.end()) {
      return false;
    }
    auto parent = m_descendants.find(i->second.m_parent);
    if(parent != m_descendants.end()) {
      std::erase(parent->second.m_children, &child);
    }
    auto children = std::move(i->second.m_children);
    m_descendants.erase(i);
    child.removeEventFilter(this);
    for(auto descendant : child.children()) {
      if(descendant && descendant->isWidgetType()) {
        remove(*descendant);
      }
    }
    for(auto descendant : children) {
      forget(descendant);
    }
    return true;
  }

  void forget(const QObject* descendant) {
    auto i = m_descendants.find(descendant);
    if(i == m_descendants.end()) {
      return;
    }
    auto children = std::move(i->second.m_children);
    m_descendants.erase(i);
    for(auto child : children) {
      forget(child);
    }
  }

  void on_position(const QPoint& position) {
    if(m_widget && (m_state != HoverObserver::State::NONE ||
        QApplication::mouseButtons() == Qt::NoButton)) {
      set_state(::get_state(*m_widget, position));
    }
  }

  void on_focus_changed(QWidget* old, QWidget* now) {
    update_state();
  }

  void on_timeout() {
    if(m_widget && m_position_observer && m_state != State::NONE) {
      set_state(::get_state(*m_widget, m_position_observer->get_position()));
    }
    if(m_state == State::NONE) {
      m_hover_timer.stop();
    }
  }
};

HoverObserver::HoverObserver(QWidget& widget) {
  m_filter = find_extension<EventFilter>(widget);
  m_filter_connection = m_filter->m_state_signal.connect(m_state_signal);
}

HoverObserver::State HoverObserver::get_state() const {
  return m_filter->m_state;
}

connection HoverObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_state_signal.connect(slot);
}

bool Spire::is_set(HoverObserver::State left, HoverObserver::State right) {
  return static_cast<HoverObserver::State>(
    static_cast<std::underlying_type_t<HoverObserver::State>>(left) &
      static_cast<std::underlying_type_t<HoverObserver::State>>(right)) !=
        HoverObserver::State::NONE;
}
