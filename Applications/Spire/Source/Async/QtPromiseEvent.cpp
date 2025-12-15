#include "Spire/Async/QtPromiseEvent.hpp"

using namespace Spire;
using namespace Spire::details;

const QEvent::Type QtDeferredExecutionEvent::EVENT_TYPE =
  static_cast<QEvent::Type>(QEvent::registerEventType());

QtDeferredExecutionEvent::QtDeferredExecutionEvent() noexcept
  : QEvent(EVENT_TYPE) {}

const QEvent::Type QtBasePromiseEvent::EVENT_TYPE =
  static_cast<QEvent::Type>(QEvent::registerEventType());

QtBasePromiseEvent::QtBasePromiseEvent() noexcept
  : QEvent(EVENT_TYPE) {}
