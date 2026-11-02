#include "Spire/Ui/HoverObserver.hpp"
#include <algorithm>
#include <functional>
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
    const QObject* m_object;
    const QObject* m_parent;
  };
  std::vector<HoverObserver*> m_observers;
  int m_depth;
  bool m_block_signal;
  QPointer<QWidget> m_widget;
  State m_state;
  std::unique_ptr<GlobalPositionObserver> m_position_observer;
  std::vector<Descendant> m_descendants;
  QTimer m_hover_timer;

  EventFilter(QWidget& widget)
      : m_depth(0),
        m_block_signal(false),
        m_widget(&widget),
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
    auto descendants = std::vector<Descendant>();
    for(auto child : m_widget->children()) {
      if(child && child->isWidgetType()) {
        collect(static_cast<QWidget&>(*child), *m_widget, descendants);
      }
    }
    insert(std::move(descendants));
    connect(
      qApp, &QApplication::focusChanged, this, &EventFilter::on_focus_changed);
  }

  void destroy_observers() {
    if(m_position_observer) {
      m_position_observer = nullptr;
      if(m_widget) {
        for(auto child : m_widget->children()) {
          if(child && child->isWidgetType()) {
            release(*child);
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
      signal_state(state);
    }
  }

  void add(HoverObserver& observer) {
    m_observers.push_back(&observer);
  }

  void remove(HoverObserver& observer) {
    auto i = std::ranges::find(m_observers, &observer);
    if(i == m_observers.end()) {
      return;
    }
    if(m_depth == 0) {
      m_observers.erase(i);
    } else {
      *i = nullptr;
    }
  }

  void signal_state(State state) {
    auto self = QPointer(this);
    auto size = m_observers.size();
    ++m_depth;
    for(auto i = std::size_t(0); i != size; ++i) {
      if(auto observer = m_observers[i]) {
        observer->m_state_signal(state);
      }
      if(!self) {
        return;
      }
    }
    --m_depth;
    if(m_depth == 0) {
      std::erase(m_observers, nullptr);
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

  std::vector<Descendant>::iterator find(const QObject& object) {
    auto i = std::ranges::lower_bound(
      m_descendants, &object, std::less(), &Descendant::m_object);
    if(i == m_descendants.end() || i->m_object != &object) {
      return m_descendants.end();
    }
    return i;
  }

  bool add(QWidget& child, const QObject& parent) {
    auto descendants = std::vector<Descendant>();
    collect(child, parent, descendants);
    if(descendants.empty()) {
      return false;
    }
    insert(std::move(descendants));
    return true;
  }

  void collect(QWidget& child, const QObject& parent,
      std::vector<Descendant>& descendants) {
    if(find(child) != m_descendants.end()) {
      return;
    }
    descendants.push_back(Descendant(&child, &parent));
    child.installEventFilter(this);
    for(auto descendant : child.children()) {
      if(descendant && descendant->isWidgetType()) {
        collect(static_cast<QWidget&>(*descendant), child, descendants);
      }
    }
  }

  void insert(std::vector<Descendant> descendants) {
    std::ranges::sort(descendants, std::less(), &Descendant::m_object);
    auto size = m_descendants.size();
    m_descendants.insert(
      m_descendants.end(), descendants.begin(), descendants.end());
    std::ranges::inplace_merge(m_descendants, m_descendants.begin() + size,
      std::less(), &Descendant::m_object);
  }

  bool remove(QObject& child) {
    if(find(child) == m_descendants.end()) {
      return false;
    }
    release(child);
    forget();
    return true;
  }

  void release(QObject& child) {
    auto i = find(child);
    if(i == m_descendants.end()) {
      return;
    }
    i->m_parent = nullptr;
    child.removeEventFilter(this);
    for(auto descendant : child.children()) {
      if(descendant && descendant->isWidgetType()) {
        release(*descendant);
      }
    }
  }

  void forget() {
    auto parents = std::vector<const QObject*>();
    for(const auto& descendant : m_descendants) {
      if(!descendant.m_parent) {
        parents.push_back(descendant.m_object);
      }
    }
    while(!parents.empty()) {
      auto children = std::vector<const QObject*>();
      for(auto& descendant : m_descendants) {
        if(descendant.m_parent &&
            std::ranges::binary_search(parents, descendant.m_parent)) {
          descendant.m_parent = nullptr;
          children.push_back(descendant.m_object);
        }
      }
      parents = std::move(children);
    }
    std::erase_if(m_descendants, [] (const auto& descendant) {
      return !descendant.m_parent;
    });
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

HoverObserver::HoverObserver(QWidget& widget)
    : m_filter(find_extension<EventFilter>(widget)) {
  m_filter->add(*this);
}

HoverObserver::~HoverObserver() {
  m_filter->remove(*this);
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
