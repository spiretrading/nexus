#ifndef SPIRE_QT_FUTURE_HPP
#define SPIRE_QT_FUTURE_HPP
#include <exception>
#include <stdexcept>
#include <utility>
#include <Beam/Routines/Async.hpp>
#include "Spire/Async/QtPromise.hpp"

namespace Spire {

  /**
   * Encapsulates a QtPromise resolving to a future value.
   * @param <T> The type of value to resolve to.
   */
  template<typename>
  class QtFuture;

  /**
   * Encapsulates a QtPromise resolving to a future value.
   * @param <T> The type of value to resolve to.
   */
  template<typename T>
  class BaseQtFuture {
    public:

      /** The type of value to resolve to. */
      using Type = T;

      /** Resolves the QtPromise waiting on this future to an exception. */
      void resolve(std::exception_ptr e);

    protected:
      Beam::Eval<Type> m_eval;

      BaseQtFuture(Beam::Eval<Type> eval) noexcept;
      BaseQtFuture(BaseQtFuture&&) = default;
      ~BaseQtFuture();

      BaseQtFuture& operator =(BaseQtFuture&& other);

    private:
      friend struct std::pair<QtFuture<Type>, QtPromise<Type>>;

      void abandon();
  };

  /**
   * Encapsulates a QtPromise resolving to a future value.
   * @param <T> The type of value to resolve to.
   */
  template<typename T>
  class QtFuture : public BaseQtFuture<T> {
    public:

      /** The type of value to resolve to. */
      using Type = T;

      QtFuture(QtFuture&&) = default;

      QtFuture& operator =(QtFuture&&) = default;
      using BaseQtFuture<T>::resolve;

      /** Resolves the QtPromise waiting on this future. */
      void resolve(Type value);

    private:
      friend struct std::pair<QtFuture, QtPromise<Type>>;

      using BaseQtFuture<T>::BaseQtFuture;
  };

  /** Specializes QtFuture for void promises. */
  template<>
  class QtFuture<void> : public BaseQtFuture<void> {
    public:
      QtFuture(QtFuture&&) = default;

      /** Resolves the QtPromise waiting on this future. */
      void resolve();

      QtFuture& operator =(QtFuture&&) = default;
      using BaseQtFuture<void>::resolve;

    private:
      friend struct std::pair<QtFuture, QtPromise<Type>>;

      using BaseQtFuture<void>::BaseQtFuture;
  };

  /**
   * Returns a pair consisting of a QtFuture<T> and its corresponding
   * QtPromise<T>.
   */
  template<typename T>
  std::pair<QtFuture<T>, QtPromise<T>> make_future() {
    auto async = std::make_unique<Beam::Async<T>>();
    auto eval = async->get_eval();
    return std::pair<QtFuture<T>, QtPromise<T>>(std::piecewise_construct,
      std::forward_as_tuple(std::move(eval)),
      std::forward_as_tuple([async = std::move(async)] {
        if constexpr(std::is_same_v<T, void>) {
          async->get();
        } else {
          return async->get();
        }
      }, LaunchPolicy::ASYNC));
  }

  template<typename T>
  void BaseQtFuture<T>::resolve(std::exception_ptr e) {
    m_eval.set_exception(std::move(e));
  }

  template<typename T>
  BaseQtFuture<T>::BaseQtFuture(Beam::Eval<Type> eval) noexcept
    : m_eval(std::move(eval)) {}

  template<typename T>
  BaseQtFuture<T>::~BaseQtFuture() {
    abandon();
  }

  template<typename T>
  BaseQtFuture<T>& BaseQtFuture<T>::operator =(BaseQtFuture&& other) {
    if(this == &other) {
      return *this;
    }
    abandon();
    m_eval = std::move(other.m_eval);
    return *this;
  }

  template<typename T>
  void BaseQtFuture<T>::abandon() {
    if(m_eval.is_empty()) {
      return;
    }
    m_eval.set_exception(
      std::make_exception_ptr(std::runtime_error("Broken future.")));
  }

  template<typename T>
  void QtFuture<T>::resolve(Type value) {
    this->m_eval.set(std::move(value));
  }

  inline void QtFuture<void>::resolve() {
    this->m_eval.set();
  }
}

#endif
