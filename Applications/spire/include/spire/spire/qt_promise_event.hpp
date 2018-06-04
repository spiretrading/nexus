#ifndef SPIRE_QT_PROMISE_EVENT_HPP
#define SPIRE_QT_PROMISE_EVENT_HPP
#include <utility>
#include <Beam/Utilities/Expect.hpp>
#include <QEvent>

namespace spire {
namespace details {
  class qt_base_promise_event : public QEvent {
    public:
      const static QEvent::Type EVENT_TYPE;

      qt_base_promise_event();
  };

  template<typename T>
  class qt_promise_event : public qt_base_promise_event {
    public:
      qt_promise_event(Beam::Expect<T> result);

      Beam::Expect<T>& get_result();

    private:
      Beam::Expect<T> m_result;
  };

  template<typename T>
  auto make_qt_promise_event(Beam::Expect<T> result) {
    return new qt_promise_event<T>(std::move(result));
  }

  template<typename T>
  qt_promise_event<T>::qt_promise_event(Beam::Expect<T> result)
      : m_result(std::move(result)) {}

  template<typename T>
  Beam::Expect<T>& qt_promise_event<T>::get_result() {
    return m_result;
  }
}
}

#endif
