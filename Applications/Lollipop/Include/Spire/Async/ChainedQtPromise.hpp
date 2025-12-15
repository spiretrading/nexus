#ifndef SPIRE_CHAINED_QT_PROMISE_HPP
#define SPIRE_CHAINED_QT_PROMISE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Spire/Async/QtPromiseImp.hpp"

namespace Spire {
  template<typename P, typename E>
  class ChainedQtPromise final : public details::BaseQtPromiseImp<
      promise_executor_result_t<E, Beam::Expect<typename P::Type>>> {
    public:
      using Promise = P;
      using Executor = E;
      using Super = details::BaseQtPromiseImp<promise_executor_result_t<
        Executor, Beam::Expect<typename Promise::Type>>>;
      using Type = typename Super::Type;
      using ContinuationType = typename Super::ContinuationType;

      template<typename ExecutorForward>
      ChainedQtPromise(Promise promise, ExecutorForward&& executor);

      void bind(std::shared_ptr<void> self);
      void then(ContinuationType continuation) override;
      void disconnect() override;

    protected:
      bool event(QEvent* event) override;

    private:
      bool m_is_disconnected;
      Promise m_promise;
      Executor m_executor;
      boost::optional<Beam::Expect<Type>> m_value;
      boost::optional<ContinuationType> m_continuation;
      std::shared_ptr<void> m_self;
  };

  template<typename Promise, typename Executor>
  auto make_chained_qt_promise(Promise&& promise, Executor&& executor) {
    return std::make_shared<
      ChainedQtPromise<std::decay_t<Promise>, std::decay_t<Executor>>>(
        std::forward<Promise>(promise), std::forward<Executor>(executor));
  }

  template<typename P, typename E>
  template<typename ExecutorForward>
  ChainedQtPromise<P, E>::ChainedQtPromise(
    Promise promise, ExecutorForward&& executor)
    : m_is_disconnected(false),
      m_promise(std::move(promise)),
      m_executor(std::forward<ExecutorForward>(executor)) {}

  template<typename P, typename E>
  void ChainedQtPromise<P, E>::bind(std::shared_ptr<void> self) {
    m_self = std::move(self);
    if constexpr(is_promise_v<
        std::invoke_result_t<Executor, Beam::Expect<typename Promise::Type>>>) {
      m_promise.finish([=, self = m_self] (auto&& result) {
        auto continuation = std::make_shared<
          std::invoke_result_t<Executor, Beam::Expect<typename Promise::Type>>>(
            m_executor(std::forward<decltype(result)>(result)));
        continuation->finish([=, continuation = continuation] (auto&& result) {
          QCoreApplication::postEvent(this, details::make_qt_promise_event(
            std::forward<decltype(result)>(result)));
        });
      });
    } else {
      m_promise.finish([=] (auto&& result) {
        using Result = decltype(std::forward<decltype(result)>(result));
        QCoreApplication::postEvent(this, details::make_qt_promise_event(
          Beam::try_call([&] {
            return m_executor(static_cast<Result>(result));
          })));
      });
    }
  }

  template<typename P, typename E>
  void ChainedQtPromise<P, E>::then(ContinuationType continuation) {
    m_continuation.emplace(std::move(continuation));
    if(m_value) {
      QCoreApplication::postEvent(
        this, details::make_qt_promise_event(std::move(*m_value)));
      m_value = boost::none;
    }
  }

  template<typename P, typename E>
  void ChainedQtPromise<P, E>::disconnect() {
    if(m_is_disconnected) {
      return;
    }
    m_is_disconnected = true;
    m_promise.disconnect();
  }

  template<typename P, typename E>
  bool ChainedQtPromise<P, E>::event(QEvent* event) {
    if(event->type() == details::QtBasePromiseEvent::EVENT_TYPE) {
      if(m_is_disconnected) {
        m_self = nullptr;
        return true;
      }
      auto& promise_event = *static_cast<details::QtPromiseEvent<Type>*>(event);
      if(m_continuation) {
        disconnect();
        auto self = std::move(m_self);
        (*m_continuation)(std::move(promise_event.get_result()));
        m_continuation = boost::none;
      } else {
        m_value = std::move(promise_event.get_result());
      }
      return true;
    } else {
      return QObject::event(event);
    }
  }
}

#endif
