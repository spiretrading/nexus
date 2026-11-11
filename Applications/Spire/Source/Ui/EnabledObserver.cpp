#include "Spire/Ui/EnabledObserver.hpp"
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <QApplication>
#include <QEvent>
#include <QPointer>
#include <QWidget>
#include "Spire/Spire/ExtensionCache.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct EnabledObserver::EventFilter : QObject {
  struct ApplicationFilter : QObject {
    std::unordered_map<const QObject*, EventFilter*> m_filters;

    static std::shared_ptr<ApplicationFilter> get_instance() {
      static auto instance = std::weak_ptr<ApplicationFilter>();
      if(auto filter = instance.lock()) {
        return filter;
      }
      auto filter = std::make_shared<ApplicationFilter>();
      instance = filter;
      return filter;
    }

    ApplicationFilter() {
      qApp->installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::EnabledChange ||
          event->type() == QEvent::ParentChange) {
        auto i = m_filters.find(watched);
        if(i != m_filters.end()) {
          auto self = QPointer(this);
          i->second->update();
          if(!self) {
            return false;
          }
        }
      }
      return QObject::eventFilter(watched, event);
    }
  };
  std::shared_ptr<ApplicationFilter> m_application_filter;
  std::vector<EnabledObserver*> m_observers;
  int m_depth;
  const QObject* m_object;
  QPointer<QWidget> m_widget;
  bool m_is_enabled;

  EventFilter(QWidget& widget)
      : m_application_filter(ApplicationFilter::get_instance()),
        m_depth(0),
        m_object(&widget),
        m_widget(&widget),
        m_is_enabled(widget.isEnabled()) {
    m_application_filter->m_filters[m_object] = this;
  }

  ~EventFilter() override {
    auto i = m_application_filter->m_filters.find(m_object);
    if(i != m_application_filter->m_filters.end() && i->second == this) {
      m_application_filter->m_filters.erase(i);
    }
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

  void update() {
    if(!m_widget || m_is_enabled == m_widget->isEnabled()) {
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
