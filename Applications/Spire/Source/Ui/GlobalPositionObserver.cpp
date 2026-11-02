#include "Spire/Ui/GlobalPositionObserver.hpp"
#include <algorithm>
#include <vector>
#include <QEvent>
#include <QPointer>
#include "Spire/Spire/ExtensionCache.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  QPoint get_position(const QWidget& widget) {
    return widget.mapToGlobal(QPoint(0, 0));
  }
}

struct GlobalPositionObserver::EventFilter : QObject {
  struct Slot {
    GlobalPositionObserver* m_observer;
    EventFilter* m_child;
  };
  std::vector<Slot> m_slots;
  int m_depth;
  QPointer<QWidget> m_widget;
  QPoint m_position;
  std::shared_ptr<EventFilter> m_parent_filter;

  EventFilter(QWidget& widget)
      : m_depth(0),
        m_widget(&widget),
        m_position(::get_position(*m_widget)) {
    m_widget->installEventFilter(this);
    connect(
      m_widget, &QObject::destroyed, this, &EventFilter::on_widget_destroyed);
    observe_parent();
  }

  ~EventFilter() {
    if(m_parent_filter) {
      m_parent_filter->remove_slot(nullptr, this);
    }
  }

  void add_slot(GlobalPositionObserver* observer, EventFilter* child) {
    m_slots.push_back(Slot(observer, child));
  }

  void remove_slot(GlobalPositionObserver* observer, EventFilter* child) {
    auto i = std::ranges::find_if(m_slots, [&] (const auto& slot) {
      return slot.m_observer == observer && slot.m_child == child;
    });
    if(i == m_slots.end()) {
      return;
    }
    if(m_depth == 0) {
      m_slots.erase(i);
    } else {
      *i = Slot(nullptr, nullptr);
    }
  }

  void set_position(QPoint position) {
    if(position == m_position) {
      return;
    }
    m_position = position;
    auto self = QPointer(this);
    auto size = m_slots.size();
    ++m_depth;
    for(auto i = std::size_t(0); i != size; ++i) {
      auto slot = m_slots[i];
      if(slot.m_observer) {
        slot.m_observer->m_position_signal(position);
      } else if(slot.m_child) {
        slot.m_child->on_position(position);
      }
      if(!self) {
        return;
      }
    }
    --m_depth;
    if(m_depth == 0) {
      std::erase_if(m_slots, [] (const auto& slot) {
        return !slot.m_observer && !slot.m_child;
      });
    }
  }

  void observe_parent() {
    if(m_parent_filter) {
      m_parent_filter->remove_slot(nullptr, this);
    }
    if(m_widget->parentWidget()) {
      m_parent_filter = find_extension<EventFilter>(*m_widget->parentWidget());
      m_parent_filter->add_slot(nullptr, this);
    } else {
      m_parent_filter = nullptr;
    }
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::Move ||
        event->type() == QEvent::LayoutRequest) {
      set_position(::get_position(*m_widget));
    } else if(event->type() == QEvent::ParentChange) {
      observe_parent();
    }
    return QObject::eventFilter(watched, event);
  }

  void on_widget_destroyed() {
    if(m_parent_filter) {
      m_parent_filter->remove_slot(nullptr, this);
      m_parent_filter = nullptr;
    }
  }

  void on_position(const QPoint& position) {
    if(m_widget) {
      set_position(::get_position(*m_widget));
    }
  }
};

GlobalPositionObserver::GlobalPositionObserver(QWidget& widget)
    : m_filter(find_extension<EventFilter>(widget)) {
  m_filter->add_slot(this, nullptr);
}

GlobalPositionObserver::~GlobalPositionObserver() {
  m_filter->remove_slot(this, nullptr);
}

QPoint GlobalPositionObserver::get_position() const {
  return m_filter->m_position;
}

connection GlobalPositionObserver::connect_position_signal(
    const PositionSignal::slot_type& slot) const {
  return m_position_signal.connect(slot);
}
