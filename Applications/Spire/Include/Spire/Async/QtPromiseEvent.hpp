#ifndef SPIRE_QT_PROMISE_EVENT_HPP
#define SPIRE_QT_PROMISE_EVENT_HPP
#include <utility>
#include <Beam/Utilities/Expect.hpp>
#include <QEvent>
#include "Spire/Async/Async.hpp"

namespace Spire {
namespace details {
  class QtDeferredExecutionEvent : public QEvent {
    public:
      const static QEvent::Type EVENT_TYPE;

      QtDeferredExecutionEvent();
  };

  class QtBasePromiseEvent : public QEvent {
    public:
      const static QEvent::Type EVENT_TYPE;

      QtBasePromiseEvent();
  };

  template<typename T>
  class QtPromiseEvent : public QtBasePromiseEvent {
    public:
      explicit QtPromiseEvent(Beam::Expect<T> result);

      Beam::Expect<T>& get_result();

    private:
      Beam::Expect<T> m_result;
  };

  template<typename T>
  auto make_qt_promise_event(Beam::Expect<T> result) {
    return new QtPromiseEvent<T>(std::move(result));
  }

  template<typename T>
  QtPromiseEvent<T>::QtPromiseEvent(Beam::Expect<T> result)
      : m_result(std::move(result)) {}

  template<typename T>
  Beam::Expect<T>& QtPromiseEvent<T>::get_result() {
    return m_result;
  }
}
}

#endif
