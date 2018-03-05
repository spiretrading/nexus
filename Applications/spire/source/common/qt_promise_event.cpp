#include "spire/spire/qt_promise_event.hpp"

using namespace spire;
using namespace spire::details;

const QEvent::Type qt_base_promise_event::EVENT_TYPE =
  static_cast<QEvent::Type>(QEvent::registerEventType());

qt_base_promise_event::qt_base_promise_event()
    : QEvent(EVENT_TYPE) {}
