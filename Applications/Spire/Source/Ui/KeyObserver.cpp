#include "Spire/Ui/KeyObserver.hpp"
#include <algorithm>
#include <vector>
#include <QPointer>
#include <QWidget>
#include "Spire/Spire/ExtensionCache.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct KeyObserver::EventFilter : QObject {
  struct Slot {
    KeyObserver* m_observer;
    EventFilter* m_parent;
  };
  struct Child {
    QObject* m_child;
    std::shared_ptr<EventFilter> m_filter;
  };
  std::vector<Slot> m_slots;
  std::vector<Child> m_children;
  int m_depth;

  EventFilter(QWidget& widget)
      : m_depth(0) {
    widget.installEventFilter(this);
    for(auto child : widget.children()) {
      if(child && child->isWidgetType()) {
        add(static_cast<QWidget&>(*child));
      }
    }
  }

  ~EventFilter() {
    for(auto& child : m_children) {
      child.m_filter->remove_slot(nullptr, this);
    }
  }

  void add_slot(KeyObserver* observer, EventFilter* parent) {
    m_slots.push_back(Slot(observer, parent));
  }

  void remove_slot(KeyObserver* observer, EventFilter* parent) {
    auto i = std::ranges::find_if(m_slots, [&] (const auto& slot) {
      return slot.m_observer == observer && slot.m_parent == parent;
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

  optional<bool> invoke(QWidget& target, QKeyEvent& event) {
    auto self = QPointer(this);
    auto result = optional<bool>();
    auto size = m_slots.size();
    ++m_depth;
    for(auto i = std::size_t(0); i != size; ++i) {
      auto slot = m_slots[i];
      if(slot.m_observer) {
        result =
          slot.m_observer->m_key_press_signal(target, event).value_or(false);
      } else if(slot.m_parent) {
        result = slot.m_parent->invoke(target, event).value_or(false);
      }
      if(!self) {
        return result;
      }
    }
    --m_depth;
    if(m_depth == 0) {
      std::erase_if(m_slots, [] (const auto& slot) {
        return !slot.m_observer && !slot.m_parent;
      });
    }
    return result;
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::KeyPress) {
      return invoke(*static_cast<QWidget*>(watched),
        *static_cast<QKeyEvent*>(event)).value_or(false);
    } else if(event->type() == QEvent::ChildAdded) {
      auto& child = *static_cast<QChildEvent&>(*event).child();
      if(child.isWidgetType()) {
        add(static_cast<QWidget&>(child));
      }
    } else if(event->type() == QEvent::ChildRemoved) {
      remove(*static_cast<QChildEvent&>(*event).child());
    }
    return QObject::eventFilter(watched, event);
  }

  void add(QWidget& child) {
    if(std::ranges::find(m_children, &child, &Child::m_child) !=
        m_children.end()) {
      return;
    }
    auto filter = find_extension<EventFilter>(child);
    filter->add_slot(nullptr, this);
    m_children.push_back(Child(&child, std::move(filter)));
  }

  void remove(QObject& child) {
    auto i = std::ranges::find(m_children, &child, &Child::m_child);
    if(i == m_children.end()) {
      return;
    }
    auto filter = std::move(i->m_filter);
    m_children.erase(i);
    filter->remove_slot(nullptr, this);
  }
};

KeyObserver::KeyObserver(QWidget& widget)
    : m_filter(find_extension<EventFilter>(widget)) {
  m_filter->add_slot(this, nullptr);
}

KeyObserver::~KeyObserver() {
  m_filter->remove_slot(this, nullptr);
}

connection KeyObserver::connect_filtered_key_press_signal(
    const FilteredKeyPressSignal::slot_type& slot) const {
  return m_key_press_signal.connect(slot);
}

connection KeyObserver::connect_key_press_signal(
    const KeyPressSignal& slot) const {
  return connect_filtered_key_press_signal([=] (auto& target, auto& event) {
    slot(target, event);
    return false;
  });
}
