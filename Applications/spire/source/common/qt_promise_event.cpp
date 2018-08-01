#include "spire/spire/qt_promise_event.hpp"

using namespace spire;
using namespace spire::details;

const QEvent::Type QtBasePromiseEvent::EVENT_TYPE =
  static_cast<QEvent::Type>(QEvent::registerEventType());

QtBasePromiseEvent::QtBasePromiseEvent()
    : QEvent(EVENT_TYPE) {}
