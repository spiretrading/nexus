#include "Spire/Styles/SelectorRegistry.hpp"
#include <deque>
#include <unordered_map>
#include <QEvent>
#include "Spire/Styles/StyledWidget.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct EventListener : QObject {
    SelectorRegistry* m_registry;

    EventListener(SelectorRegistry& registry)
      : m_registry(&registry) {}

    bool eventFilter(QObject* watched, QEvent* event) override {
      auto& widget = *static_cast<QWidget*>(watched);
      if(event->type() == QEvent::WindowActivate ||
          event->type() == QEvent::WindowDeactivate) {
        if(widget.isWindow()) {
          m_registry->notify();
        }
      } else if(event->type() == QEvent::FocusIn ||
          event->type() == QEvent::FocusOut ||
          event->type() == QEvent::Enter ||
          event->type() == QEvent::Leave ||
          event->type() == QEvent::EnabledChange) {
        m_registry->notify();
      } else if(event->type() == QEvent::ChildAdded ||
          event->type() == QEvent::ChildRemoved) {
        auto& child_event = *static_cast<QChildEvent*>(event);
        if(auto child = qobject_cast<QWidget*>(child_event.child())) {
          if(child_event.added()) {
            child->installEventFilter(this);
          } else {
            child->removeEventFilter(this);
          }
          m_registry->notify();
        }
      } else if(event->type() == QEvent::ParentChange) {
        if(widget.window() != &m_registry->get_root()) {
          widget.removeEventFilter(this);
          SelectorRegistry::add(widget);
        }
      }
      return QObject::eventFilter(watched, event);
    }
  };

  std::unordered_map<QWidget*, std::unique_ptr<SelectorRegistry>> registries;

  template<typename F>
  void for_each_bfs(QWidget& root, F&& f) {
    auto descendants = std::deque<QWidget*>();
    descendants.push_back(&root);
    while(!descendants.empty()) {
      auto descendant = descendants.front();
      descendants.pop_front();
      std::forward<F>(f)(*descendant);
      for(auto child : descendant->children()) {
        if(auto widget = qobject_cast<QWidget*>(child)) {
          descendants.push_back(widget);
        }
      }
    }
  }
}

SelectorRegistry& SelectorRegistry::find(QWidget& widget) {
  auto window = widget.window();
  auto i = registries.find(window);
  if(i == registries.end()) {
    i = registries.emplace_hint(i, window,
      std::unique_ptr<SelectorRegistry>(new SelectorRegistry(*window)));
  }
  return *i->second;
}

void SelectorRegistry::add(QWidget& widget) {
  find(widget);
}

QWidget& SelectorRegistry::get_root() const {
  return *m_root;
}

void SelectorRegistry::notify() {
  for_each_bfs(*m_root, [&] (QWidget& widget) {
    if(auto styled_widget = dynamic_cast<StyledWidget*>(&widget)) {
      styled_widget->selector_updated();
    }
  });
}

SelectorRegistry::SelectorRegistry(QWidget& root)
    : m_root(&root),
      m_event_listener(std::make_unique<EventListener>(*this)) {
  for_each_bfs(*m_root, [&] (QWidget& widget) {
    widget.installEventFilter(m_event_listener.get());
  });
}
