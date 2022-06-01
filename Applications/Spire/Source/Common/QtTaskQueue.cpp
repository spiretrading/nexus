#include "Spire/Spire/QtTaskQueue.hpp"
#include <mutex>
#include <Beam/Routines/RoutineHandler.hpp>
#include <QCoreApplication>
#include <QEvent>

using namespace Beam;
using namespace Beam::Routines;
using namespace Spire;

namespace {
  struct QtTaskEvent : QEvent {
    const static QEvent::Type EVENT_TYPE;

    QtTaskEvent()
      : QEvent(EVENT_TYPE) {}
  };

  const QEvent::Type QtTaskEvent::EVENT_TYPE =
    static_cast<QEvent::Type>(QEvent::registerEventType());
}

struct QtTaskQueue::EventHandler : QObject {
  mutable std::mutex m_mutex;
  std::vector<std::function<void ()>> m_tasks;
  RoutineHandler m_routine;

  void push(const Target& value) {
    auto is_empty = [&] {
      auto lock = std::lock_guard(m_mutex);
      m_tasks.push_back(value);
      return m_tasks.size() == 1;
    }();
    if(is_empty) {
      QCoreApplication::postEvent(this, new QtTaskEvent());
    }
  }

  void push(Target&& value) {
    auto is_empty = [&] {
      auto lock = std::lock_guard(m_mutex);
      m_tasks.push_back(std::move(value));
      return m_tasks.size() == 1;
    }();
    if(is_empty) {
      QCoreApplication::postEvent(this, new QtTaskEvent());
    }
  }

  bool event(QEvent* event) override {
    if(event->type() != QtTaskEvent::EVENT_TYPE) {
      return QObject::event(event);
    }
    auto tasks = [&] {
      auto lock = std::lock_guard(m_mutex);
      return std::move(m_tasks);
    }();
    for(auto& task : tasks) {
      try {
        task();
      } catch(const std::exception&) {
      }
    }
    return QObject::event(event);
  }
};

QtTaskQueue::QtTaskQueue()
  : m_is_broken(false),
    m_event_handler(std::make_unique<EventHandler>()) {}

QtTaskQueue::~QtTaskQueue() {
  close();
}

void QtTaskQueue::push(const Target& value) {
  Push(value);
}

void QtTaskQueue::push(Target&& value) {
  Push(std::move(value));
}

void QtTaskQueue::close() {
  Break();
}

void QtTaskQueue::close(const std::exception_ptr& exception) {
  Break(exception);
}

void QtTaskQueue::safe_push(Target&& value) {
  m_event_handler->push(std::move(value));
}

void QtTaskQueue::Push(const Target& value) {
  if(m_is_broken) {
    std::rethrow_exception(m_break_exception);
  }
  m_event_handler->push(value);
}

void QtTaskQueue::Push(Target&& value) {
  if(m_is_broken) {
    std::rethrow_exception(m_break_exception);
  }
  m_event_handler->push(std::move(value));
}

void QtTaskQueue::Break(const std::exception_ptr& exception) {
  if(m_is_broken.exchange(true)) {
    return;
  }
  m_break_exception = exception;
  m_callbacks.Break(exception);
}
