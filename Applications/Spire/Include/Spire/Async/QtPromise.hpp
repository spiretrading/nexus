#ifndef SPIRE_QT_PROMISE_HPP
#define SPIRE_QT_PROMISE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <QApplication>
#include "Spire/Async/ChainedQtPromise.hpp"
#include "Spire/Async/QtPromiseImp.hpp"

namespace Spire {

  /**
   * Performs an asyncronous computation and signals the result within a
   * Qt-thread in a thread-safe manner.
   * @param <T> The type of value to compute.
   */
  template<typename T>
  class QtPromise {
    public:

      /** The type of value to compute. */
      using Type = T;

      /** Constructs an empty promise. */
      QtPromise();

      /**
       * Constructs a Qt promise.
       * @param executor The callable performing the computation.
       * @param launch_policy Specifies how the executor should be invoked.
       */
      template<std::invocable F>
      explicit QtPromise(
        F&& executor, LaunchPolicy launch_policy = LaunchPolicy::DEFERRED);

      /**
       * Constructs an immediate Qt promise.
       * @param value The value to evaluate to.
       */
      template<typename U> requires(!std::invocable<U>)
      QtPromise(U&& value);

      /**
       * Constructs an immediate Qt promise.
       * @param value The value to evaluate to.
       */
      template<typename U>
      QtPromise(Beam::Expect<U> value);

      template<typename U> requires std::same_as<T, void>
      QtPromise(QtPromise<U>&& other);
      QtPromise(QtPromise&& other);
      ~QtPromise();

      /**
       * Assigns a function to be called when the computation completes.
       * @param continuation The function to call when the computation
       *        completes.
       */
      template<std::invocable<Beam::Expect<T>> F>
      QtPromise<promise_executor_result_t<F, Beam::Expect<T>>> then(
        F&& continuation);

      /**
       * Disconnects from this promise, upon disconnection the callback will not
       * be called when the executor completes its computation.
       */
      void disconnect();

      /** Disconnects this promise and then moves another promise into this. */
      QtPromise& operator =(QtPromise&& other);

    private:
      template<typename> friend class QtPromise;
      template<typename, typename> friend class ChainedQtPromise;
      std::shared_ptr<details::BaseQtPromiseImp<Type>> m_imp;

      QtPromise(const QtPromise&) = delete;
      QtPromise& operator =(const QtPromise&) = delete;
      template<typename U, typename F>
      QtPromise(QtPromise<U> promise, F&& continuation);
      template<typename F>
      void finish(F&& continuation);
  };

  template<typename F, typename = std::enable_if_t<std::is_invocable_v<F>>>
  QtPromise(F&&) -> QtPromise<promise_executor_result_t<F>>;

  template<typename F, typename = std::enable_if_t<std::is_invocable_v<F>>>
  QtPromise(F&&, LaunchPolicy) -> QtPromise<promise_executor_result_t<F>>;

  template<typename U, typename = std::enable_if_t<!std::is_invocable_v<U>>>
  QtPromise(U&&) -> QtPromise<std::remove_cvref_t<U>>;

  template<typename U>
  QtPromise(Beam::Expect<U>) -> QtPromise<U>;

  QtPromise() -> QtPromise<void>;

  /**
   * Returns a promise that signals the result only when all provided promises
   * have completed, or throws an exception if any provided promise throws an
   * exception.
   * @param promises The promises to be executed.
   */
  template<typename T>
  QtPromise<std::vector<T>> all(std::vector<QtPromise<T>> promises) {
    if(promises.empty()) {
      return std::vector<T>();
    }
    auto completed_promises = std::make_unique<std::vector<T>>();
    auto promise = std::move(promises.front());
    for(auto i = std::size_t(0); i < promises.size() - 1; ++i) {
      promise = promise.then([=, p = std::move(promises[i + 1]),
          completed_promises = completed_promises.get()]
        (auto&& result) mutable {
          completed_promises->push_back(
            std::forward<decltype(result)>(result).get());
          return std::move(p);
        });
    }
    return promise.then(
      [=, completed_promises = std::move(completed_promises)] (
          auto&& result) mutable {
        completed_promises->push_back(
          std::forward<decltype(result)>(result).get());
        return std::move(*completed_promises);
      });
  }

  /** Specialization of all for QtPromise<void>. */
  QtPromise<void> all(std::vector<QtPromise<void>> promises);

  /**
   * Waits for a promise to complete and returns its result.
   * @param promise The promise to wait for.
   * @return The result of the promise's execution.
   */
  template<typename T>
  T wait(QtPromise<T>& promise) {
    auto future = std::optional<Beam::Expect<T>>();
    auto continuation = promise.then([&] (auto&& result) {
      future.emplace(std::forward<decltype(result)>(result));
    });
    while(!future.has_value()) {
      QApplication::processEvents(QEventLoop::WaitForMoreEvents);
      QCoreApplication::sendPostedEvents();
    }
    if constexpr(std::is_same_v<T, void>) {
      return future->get();
    } else {
      return std::move(future->get());
    }
  }

  /**
   * Waits for a promise to complete and returns its result.
   * @param promise The promise to wait for.
   * @return The result of the promise's execution.
   */
  template<typename T>
  T wait(QtPromise<T> promise) {
    return static_cast<T (*)(QtPromise<T>&)>(wait)(promise);
  }

  template<typename T>
  QtPromise<T>::QtPromise() {
    if constexpr(std::is_same_v<T, void>) {
      auto executor = [] {};
      using Executor = decltype(executor);
      auto imp = std::make_shared<details::QtPromiseImp<Executor>>(
        executor, LaunchPolicy::ASYNC);
      imp->bind(imp);
      m_imp = std::move(imp);
    }
  }

  template<typename T>
  template<std::invocable F>
  QtPromise<T>::QtPromise(F&& executor, LaunchPolicy launch_policy) {
    auto imp = std::make_shared<details::QtPromiseImp<F>>(
      std::forward<F>(executor), launch_policy);
    imp->bind(imp);
    m_imp = std::move(imp);
  }

  template<typename T>
  template<typename U> requires(!std::invocable<U>)
  QtPromise<T>::QtPromise(U&& value)
    : QtPromise([value = std::forward<U>(value)] () mutable {
        return std::move(value);
      }) {}

  template<typename T>
  template<typename U>
  QtPromise<T>::QtPromise(Beam::Expect<U> value)
    : QtPromise([value = std::move(value)] () mutable {
        return std::move(value.Get());
      }) {}

  template<typename T>
  QtPromise<T>::QtPromise(QtPromise&& other)
    : m_imp(std::move(other.m_imp)) {}

  template<typename T>
  QtPromise<T>::~QtPromise() {
    disconnect();
  }

  template<typename T>
  template<std::invocable<Beam::Expect<T>> F>
  QtPromise<promise_executor_result_t<F, Beam::Expect<T>>> QtPromise<T>::then(
      F&& continuation) {
    return QtPromise<promise_executor_result_t<F, Beam::Expect<T>>>(
      std::move(*this), std::forward<F>(continuation));
  }

  template<typename T>
  void QtPromise<T>::disconnect() {
    if(m_imp == nullptr) {
      return;
    }
    m_imp->disconnect();
    m_imp = nullptr;
  }

  template<typename T>
  QtPromise<T>& QtPromise<T>::operator =(QtPromise&& other) {
    disconnect();
    m_imp = std::move(other.m_imp);
    return *this;
  }

  template<typename T>
  template<typename U, typename F>
  QtPromise<T>::QtPromise(QtPromise<U> promise, F&& continuation) {
    auto chain = make_chained_qt_promise(std::move(promise),
      std::forward<F>(continuation));
    chain->bind(chain);
    m_imp = std::move(chain);
  }

  template<typename T>
  template<typename F>
  void QtPromise<T>::finish(F&& continuation) {
    m_imp->then(std::forward<F>(continuation));
  }
}

#endif
