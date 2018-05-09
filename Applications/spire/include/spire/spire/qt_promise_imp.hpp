#ifndef SPIRE_QT_PROMISE_IMP_HPP
#define SPIRE_QT_PROMISE_IMP_HPP
#include <functional>
#include <memory>
#include <utility>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <QApplication>
#include <QEvent>
#include <QObject>
#include "spire/spire/spire.hpp"
#include "spire/spire/qt_promise_event.hpp"

namespace spire {
namespace details {
  template<typename T>
  class base_qt_promise_imp : public QObject, private boost::noncopyable {
    public:
      using type = T;
      using continuation_type = std::function<void (Beam::Expect<type> value)>;

      virtual ~base_qt_promise_imp() = default;

      virtual void then(continuation_type continuation) = 0;

      virtual void disconnect() = 0;
  };

  template<typename Executor>
  class qt_promise_imp final :
      public base_qt_promise_imp<std::result_of_t<Executor()>> {
    public:
      using super = base_qt_promise_imp<std::result_of_t<Executor()>>;
      using type = typename super::type;
      using continuation_type = typename super::continuation_type;

      template<typename ExecutorForward>
      qt_promise_imp(ExecutorForward&& executor);

      ~qt_promise_imp() override;

      void bind(std::shared_ptr<void> self);

      void then(continuation_type continuation) override;

      void disconnect() override;

    protected:
      bool event(QEvent* event) override;

    private:
      bool m_is_disconnected;
      Executor m_executor;
      boost::optional<Beam::Expect<type>> m_value;
      boost::optional<continuation_type> m_continuation;
      std::shared_ptr<void> m_self;
      Beam::Routines::RoutineHandler m_routine;
  };

  template<typename Executor>
  template<typename ExecutorForward>
  qt_promise_imp<Executor>::qt_promise_imp(ExecutorForward&& executor)
      : m_is_disconnected(false),
        m_executor(std::forward<ExecutorForward>(executor)) {}

  template<typename Executor>
  qt_promise_imp<Executor>::~qt_promise_imp() {
    disconnect();
  }

  template<typename Executor>
  void qt_promise_imp<Executor>::bind(std::shared_ptr<void> self) {
    m_self = std::move(self);
    m_routine = Beam::Routines::Spawn(
      [=] {
        QApplication::instance()->postEvent(this,
          make_qt_promise_event(Beam::Try(m_executor)));
      });
  }

  template<typename Executor>
  void qt_promise_imp<Executor>::then(continuation_type continuation) {
    m_continuation.emplace(std::move(continuation));
    if(m_value.is_initialized()) {
      QApplication::instance()->postEvent(this,
        details::make_qt_promise_event(std::move(*m_value)));
      m_value = boost::none;
    }
  }

  template<typename Executor>
  void qt_promise_imp<Executor>::disconnect() {
    m_is_disconnected = true;
  }

  template<typename Executor>
  bool qt_promise_imp<Executor>::event(QEvent* event) {
    if(event->type() != qt_base_promise_event::EVENT_TYPE) {
      return QObject::event(event);
    }
    if(m_is_disconnected) {
      m_self = nullptr;
      return true;
    }
    auto& promise_event = *static_cast<qt_promise_event<type>*>(event);
    if(m_continuation.is_initialized()) {
      (*m_continuation)(std::move(promise_event.get_result()));
      disconnect();
      m_self = nullptr;
    } else {
      m_value = std::move(promise_event.get_result());
    }
    return true;
  }
}
}

#endif
