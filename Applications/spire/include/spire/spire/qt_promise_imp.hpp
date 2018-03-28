#ifndef SPIRE_QT_PROMISE_IMP_HPP
#define SPIRE_QT_PROMISE_IMP_HPP
#include <memory>
#include <utility>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <QApplication>
#include <QEvent>
#include <QObject>
#include "spire/spire/spire.hpp"
#include "spire/spire/qt_promise_event.hpp"

namespace spire {
namespace details {
  class base_qt_promise_imp : private boost::noncopyable {
    public:
      virtual ~base_qt_promise_imp() = default;

      virtual void disconnect() = 0;
  };

  template<typename Executor, typename Callback>
  class qt_promise_imp : public base_qt_promise_imp, public QObject {
    public:
      template<typename ExecutorForward, typename CallbackForward>
      qt_promise_imp(ExecutorForward&& executor, CallbackForward&& callback);

      ~qt_promise_imp() final override;

      void bind(std::shared_ptr<void> self);

      void disconnect() final override;

    protected:
      bool event(QEvent* event) final override;

    private:
      bool m_is_disconnected;
      Executor m_executor;
      Callback m_callback;
      std::shared_ptr<void> m_self;
      Beam::Routines::RoutineHandler m_routine;
  };

  template<typename Executor, typename Callback>
  template<typename ExecutorForward, typename CallbackForward>
  qt_promise_imp<Executor, Callback>::qt_promise_imp(ExecutorForward&& executor,
      CallbackForward&& callback)
      : m_is_disconnected(false),
        m_executor(std::forward<ExecutorForward>(executor)),
        m_callback(std::forward<CallbackForward>(callback)) {
  }

  template<typename Executor, typename Callback>
  qt_promise_imp<Executor, Callback>::~qt_promise_imp() {
    disconnect();
  }

  template<typename Executor, typename Callback>
  void qt_promise_imp<Executor, Callback>::bind(std::shared_ptr<void> self) {
    m_self = std::move(self);
    m_routine = Beam::Routines::Spawn(
      [=] {
        QApplication::instance()->postEvent(this,
          make_qt_promise_event(Beam::Try(m_executor)));
      });
  }

  template<typename Executor, typename Callback>
  void qt_promise_imp<Executor, Callback>::disconnect() {
    m_is_disconnected = true;
  }

  template<typename Executor, typename Callback>
  bool qt_promise_imp<Executor, Callback>::event(QEvent* event) {
    if(event->type() != qt_base_promise_event::EVENT_TYPE) {
      return QObject::event(event);
    }
    if(m_is_disconnected) {
      m_self.reset();
      return true;
    }
    using Type = std::result_of_t<Executor()>;
    auto& promise_event = *static_cast<qt_promise_event<Type>*>(event);
    m_callback(std::move(promise_event.get_result()));
    disconnect();
    m_self.reset();
    return true;
  }
}
}

#endif
