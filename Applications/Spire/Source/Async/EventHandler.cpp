#include "Spire/Async/EventHandler.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QCoreApplication>
#include <QEvent>
#include <QThread>
#include <QTimer>

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Spire;

namespace {
  constexpr auto UPDATE_INTERVAL = 100;

  struct EventHandlerEvent : QEvent {
    inline static const auto EventType =
      static_cast<QEvent::Type>(QEvent::registerEventType());

    EventHandlerEvent()
      : QEvent(EventType) {}
  };

  struct GlobalEventHandler : QObject {
    std::vector<std::weak_ptr<TaskQueue>> m_tasks;
    QTimer m_timer;

    GlobalEventHandler() {
      if(QThread::currentThread() == QCoreApplication::instance()->thread()) {
        start_timer();
      } else {
        QCoreApplication::postEvent(this, new EventHandlerEvent());
      }
    }

    static GlobalEventHandler& get_instance() {
      static auto instance = GlobalEventHandler();
      return instance;
    }

    bool event(QEvent* event) override {
      if(event->type() == EventHandlerEvent::EventType) {
        start_timer();
      }
      return QObject::event(event);
    }

    void start_timer() {
      connect(
        &m_timer, &QTimer::timeout, this, &GlobalEventHandler::on_expired);
      m_timer.start(UPDATE_INTERVAL);
    }

    void update(std::shared_ptr<TaskQueue> tasks) {
      auto start = microsec_clock::universal_time();
      for(auto task = tasks->TryPop(); task && tasks.use_count() != 1;
          task = tasks->TryPop()) {
        (*task)();
        auto duration = microsec_clock::universal_time();
        if(duration - start > seconds(1) / 10) {
          return;
        }
      }
    }

    void on_expired() {
      auto i = m_tasks.begin();
      while(i != m_tasks.end()) {
        if(auto tasks = i->lock()) {
          if(i != m_tasks.begin()) {
            QCoreApplication::instance()->processEvents();
          }
          update(std::move(tasks));
          ++i;
        } else {
          i = m_tasks.erase(i);
        }
      }
    }
  };
}

EventHandler::EventHandler()
    : m_tasks(std::make_shared<TaskQueue>()) {
  GlobalEventHandler::get_instance().m_tasks.push_back(m_tasks);
}
