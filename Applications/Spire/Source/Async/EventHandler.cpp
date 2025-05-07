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
    static inline auto TIME_SLICE = seconds(1) / 10;
    std::vector<std::weak_ptr<TaskQueue>> m_tasks;
    QTimer m_timer;

    GlobalEventHandler() {
      if(QCoreApplication::instance() &&
          QThread::currentThread() == QCoreApplication::instance()->thread()) {
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

    time_duration update(std::shared_ptr<TaskQueue> tasks) {
      auto start = microsec_clock::universal_time();
      for(auto task = tasks->TryPop(); task && tasks.use_count() != 1;
          task = tasks->TryPop()) {
        (*task)();
        auto duration = microsec_clock::universal_time() - start;
        if(duration >= TIME_SLICE) {
          return duration;
        }
      }
      return microsec_clock::universal_time() - start;
    }

    void on_expired() {
      auto i = std::size_t(0);
      auto running_time = seconds(0);
      while(i < m_tasks.size()) {
        if(running_time >= TIME_SLICE) {
          QCoreApplication::instance()->processEvents();
          running_time = seconds(0);
        }
        if(auto tasks = m_tasks[i].lock()) {
          running_time += update(std::move(tasks));
          ++i;
        } else {
          m_tasks.erase(m_tasks.begin() + i);
        }
      }
    }
  };
}

EventHandler::EventHandler()
    : m_tasks(std::make_shared<TaskQueue>()) {
  GlobalEventHandler::get_instance().m_tasks.push_back(m_tasks);
}
