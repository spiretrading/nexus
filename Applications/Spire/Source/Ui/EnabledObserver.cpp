#include "Spire/Ui/EnabledObserver.hpp"
#include <algorithm>
#include <vector>
#include <QEvent>
#include <QPointer>
#include <QWidget>
#include "Spire/Spire/ExtensionCache.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct EnabledObserver::EventFilter : QObject {
  std::vector<EnabledObserver*> m_observers;
  int m_depth;
  QPointer<QWidget> m_widget;
  bool m_is_enabled;

  EventFilter(QWidget& widget)
      : m_depth(0),
        m_widget(&widget),
        m_is_enabled(widget.isEnabled()) {
    m_widget->installEventFilter(this);
  }

  void add(EnabledObserver& observer) {
    m_observers.push_back(&observer);
  }

  void remove(EnabledObserver& observer) {
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

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::EnabledChange ||
        event->type() == QEvent::ParentChange) {
      update();
    }
    return QObject::eventFilter(watched, event);
  }

  void update() {
    if(m_is_enabled == m_widget->isEnabled()) {
      return;
    }
    m_is_enabled = m_widget->isEnabled();
    auto is_enabled = m_is_enabled;
    auto self = QPointer(this);
    auto size = m_observers.size();
    ++m_depth;
    for(auto i = std::size_t(0); i != size; ++i) {
      if(auto observer = m_observers[i]) {
        observer->m_enabled_signal(is_enabled);
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
};

EnabledObserver::EnabledObserver(QWidget& widget)
    : m_filter(find_extension<EventFilter>(widget)) {
  m_filter->add(*this);
}

EnabledObserver::~EnabledObserver() {
  m_filter->remove(*this);
}

connection EnabledObserver::connect_enabled_signal(
    const EnabledSignal::slot_type& slot) const {
  return m_enabled_signal.connect(slot);
}
