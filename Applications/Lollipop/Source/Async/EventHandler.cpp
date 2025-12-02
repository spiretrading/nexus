#include "Spire/Async/EventHandler.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QCoreApplication>
#include <QEvent>
#include <QThread>

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

  struct TimerDeleter : QObject {
    std::unique_ptr<QTimer> m_timer;

    TimerDeleter(std::unique_ptr<QTimer> timer)
      : m_timer(std::move(timer)) {}

    bool event(QEvent* event) {
      if(event->type() == EventHandlerEvent::EventType) {
        m_timer = nullptr;
        deleteLater();
      }
      return QObject::event(event);
    }
  };
}

EventHandler::EventHandler()
    : m_tasks(std::make_shared<TaskQueue>()) {
  if(QThread::currentThread() == QCoreApplication::instance()->thread()) {
    start_timer();
  } else {
    QCoreApplication::postEvent(this, new EventHandlerEvent());
  }
}

EventHandler::~EventHandler() {
  if(QCoreApplication::instance() != nullptr &&
      QThread::currentThread() != QCoreApplication::instance()->thread()) {
    auto deleter = new TimerDeleter(std::move(m_update_timer));
    QCoreApplication::postEvent(deleter, new EventHandlerEvent());
  }
}

bool EventHandler::event(QEvent* event) {
  if(event->type() == EventHandlerEvent::EventType) {
    start_timer();
  }
  return QObject::event(event);
}

void EventHandler::start_timer() {
  m_update_timer = std::make_unique<QTimer>();
  connect(
    m_update_timer.get(), &QTimer::timeout, this, &EventHandler::on_expired);
  m_update_timer->start(UPDATE_INTERVAL);
}

void EventHandler::on_expired() {
  auto start = microsec_clock::universal_time();
  auto tasks = m_tasks;
  for(auto task = tasks->try_pop(); task && tasks.use_count() != 1;
      task = tasks->try_pop()) {
    (*task)();
    auto duration = microsec_clock::universal_time();
    if(duration - start > seconds(1) / 10) {
      QCoreApplication::instance()->processEvents();
      start = microsec_clock::universal_time();
    }
  }
}
