#include "Spire/Spire/QtTaskQueue.hpp"
#include <QEvent>

using namespace Spire;

namespace {
  struct QtTaskEvent : QEvent {
    const static QEvent::Type EVENT_TYPE;

    QtTaskEvent();
  };

  const QEvent::Type QtTaskEvent::EVENT_TYPE =
    static_cast<QEvent::Type>(QEvent::registerEventType());

  QtTaskEvent::QtTaskEvent()
    : QEvent(EVENT_TYPE) {}
}

struct QtTaskQueue::EventHandler : QObject {
  bool event(QEvent* event) override {
    if(event->type() != QtTaskEvent::EVENT_TYPE) {
      return QObject::event(event);
    }
    return QObject::event(event);
  }
};

QtTaskQueue::QtTaskQueue()
  : m_event_handler(std::make_unique<EventHandler>()) {}

QtTaskQueue::~QtTaskQueue() {}

void QtTaskQueue::wait() {}

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

void QtTaskQueue::Push(const Target& value) {
}

void QtTaskQueue::Push(Target&& value) {
}

void QtTaskQueue::Break(const std::exception_ptr& exception) {
  m_tasks.Break(exception);
}
