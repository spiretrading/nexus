#ifndef SPIRE_QT_PROMISE_IMP_HPP
#define SPIRE_QT_PROMISE_IMP_HPP
#include <functional>
#include <memory>
#include <utility>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/optional.hpp>
#include <QCoreApplication>
#include <QEvent>
#include <QObject>
#include "Spire/Async/QtPromiseEvent.hpp"

namespace Spire {

  /** Specifies how a QtPromise's executor should be invoked. */
  enum class LaunchPolicy {

    /** The executor should be invoked within Qt's event loop. */
    DEFERRED,

    /** The executor should be invoked asynchronously. */
    ASYNC
  };

  template<typename T>
  class QtPromise;

  /** Type trait used to determine if a type represents a promise. */
  template<typename T>
  struct is_promise : std::false_type {};

  template<typename T>
  struct is_promise<QtPromise<T>> : std::true_type {};

  template<typename T>
  inline constexpr auto is_promise_v = is_promise<T>::value;

  /** Type trait used to determine the type a promise evaluates to. */
  template<typename T>
  struct promise_result {
    using type = T;
  };

  template<typename T>
  struct promise_result<QtPromise<T>> {
    using type = T;
  };

  template<typename T>
  using promise_result_t = typename promise_result<T>::type;

  /**
   * Type trait used to determine the type of promise represented by an
   * executor.
   */
  template<typename T, typename... A>
  struct promise_executor_result {
    using type = promise_result_t<std::invoke_result_t<T, A...>>;
  };

  template<typename T, typename... A>
  using promise_executor_result_t =
    typename promise_executor_result<T, A...>::type;

namespace details {
  template<typename T>
  struct is_promise_chained : std::false_type {};

  template<typename T>
  struct is_promise_chained<QtPromise<T>> : std::true_type {};

  template<typename T>
  inline constexpr auto is_promise_chained_v = is_promise_chained<T>::value;

  template<typename T>
  class BaseQtPromiseImp : public QObject {
    public:
      using QObject::QObject;
      using Type = T;
      using ContinuationType = std::function<void (Beam::Expect<Type> value)>;

      virtual ~BaseQtPromiseImp() = default;

      virtual void then(ContinuationType continuation) = 0;
      virtual void disconnect() = 0;

    private:
      BaseQtPromiseImp(const BaseQtPromiseImp&) = delete;
      BaseQtPromiseImp& operator =(const BaseQtPromiseImp&) = delete;
  };

  template<>
  class BaseQtPromiseImp<void> : public QObject {
    public:
      using QObject::QObject;
      using Type = void;
      using ContinuationType = std::function<void (Beam::Expect<void> value)>;

      virtual ~BaseQtPromiseImp() = default;

      virtual void then(ContinuationType continuation) = 0;
      virtual void disconnect() = 0;

    private:
      BaseQtPromiseImp(const BaseQtPromiseImp&) = delete;
      BaseQtPromiseImp& operator =(const BaseQtPromiseImp&) = delete;
  };

  template<typename Executor>
  class QtPromiseImp final :
      public BaseQtPromiseImp<promise_executor_result_t<Executor>> {
    public:
      using Super = BaseQtPromiseImp<promise_executor_result_t<Executor>>;
      using Type = typename Super::Type;
      using ContinuationType = typename Super::ContinuationType;

      template<typename ExecutorForward>
      QtPromiseImp(ExecutorForward&& executor, LaunchPolicy launch_policy);
      ~QtPromiseImp() override;

      void bind(std::shared_ptr<void> self);
      void then(ContinuationType continuation) override;
      void disconnect() override;

    protected:
      bool event(QEvent* event) override;

    private:
      bool m_is_disconnected;
      Executor m_executor;
      LaunchPolicy m_launch_policy;
      boost::optional<Beam::Expect<Type>> m_value;
      boost::optional<ContinuationType> m_continuation;
      std::shared_ptr<void> m_self;
      Beam::RoutineHandler m_routine;

      void execute();
  };

  template<typename Executor>
  template<typename ExecutorForward>
  QtPromiseImp<Executor>::QtPromiseImp(
    ExecutorForward&& executor, LaunchPolicy launch_policy)
    : m_is_disconnected(false),
      m_executor(std::forward<ExecutorForward>(executor)),
      m_launch_policy(launch_policy) {}

  template<typename Executor>
  QtPromiseImp<Executor>::~QtPromiseImp() {
    disconnect();
  }

  template<typename Executor>
  void QtPromiseImp<Executor>::bind(std::shared_ptr<void> self) {
    m_self = std::move(self);
    if(m_launch_policy == LaunchPolicy::DEFERRED) {
      QCoreApplication::postEvent(this, new QtDeferredExecutionEvent());
    } else if(m_launch_policy == LaunchPolicy::ASYNC) {
      m_routine = Beam::spawn([=] { execute(); });
    }
  }

  template<typename Executor>
  void QtPromiseImp<Executor>::then(ContinuationType continuation) {
    m_continuation.emplace(std::move(continuation));
    if(m_value) {
      QCoreApplication::postEvent(
        this, make_qt_promise_event(std::move(*m_value)));
      m_value = boost::none;
    }
  }

  template<typename Executor>
  void QtPromiseImp<Executor>::disconnect() {
    m_is_disconnected = true;
  }

  template<typename Executor>
  bool QtPromiseImp<Executor>::event(QEvent* event) {
    if(event->type() == QtDeferredExecutionEvent::EVENT_TYPE) {
      auto result = Beam::try_call(m_executor);
      if(m_is_disconnected) {
        m_self = nullptr;
        return true;
      }
      if(m_continuation) {
        disconnect();
        auto self = std::move(m_self);
        (*m_continuation)(std::move(result));
        m_continuation = boost::none;
      } else {
        m_value.emplace(std::move(result));
      }
      return true;
    } else if(event->type() == QtBasePromiseEvent::EVENT_TYPE) {
      if(m_is_disconnected) {
        m_self = nullptr;
        return true;
      }
      auto& promise_event = *static_cast<QtPromiseEvent<Type>*>(event);
      if(m_continuation) {
        disconnect();
        auto self = std::move(m_self);
        (*m_continuation)(std::move(promise_event.get_result()));
        m_continuation = boost::none;
      } else {
        m_value.emplace(std::move(promise_event.get_result()));
      }
      return true;
    } else {
      return QObject::event(event);
    }
  }

  template<typename Executor>
  void QtPromiseImp<Executor>::execute() {
    using Result = std::invoke_result_t<Executor>;
    if constexpr(is_promise_chained_v<Result>) {
      auto result = Beam::try_call(m_executor);
      if(result.is_exception()) {
        QCoreApplication::postEvent(this,
          make_qt_promise_event(Beam::Expect<Type>(result.GetException())));
      } else {
        auto promise = std::make_shared<Result>(std::move(result.get()));
        promise->then(
          [=, promise = std::move(promise)] (auto&& result) mutable {
            QCoreApplication::postEvent(this,
              make_qt_promise_event(std::forward<decltype(result)>(result)));
            promise.reset();
          });
      }
    } else {
      QCoreApplication::postEvent(
        this, make_qt_promise_event(Beam::try_call(m_executor)));
    }
  }
}
}

#endif
