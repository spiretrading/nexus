#ifndef SPIRE_QT_FUTURE_HPP
#define SPIRE_QT_FUTURE_HPP
#include <exception>
#include <utility>
#include <Beam/Routines/Async.hpp>
#include <boost/noncopyable.hpp>
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \brief Encapsulates a QtPromise resolving to a future value.
      \tparam T The type of value to resolve to.
  */
  template<typename T>
  class QtFuture : private boost::noncopyable {
    public:

      //! The type of value to resolve to.
      using Type = T;

      QtFuture(QtFuture&&) = default;

      QtFuture& operator =(QtFuture&&) = default;

      //! Resolves the QtPromise waiting on this future.
      void resolve(Type value);

      //! Resolves the QtPromise waiting on this future to an exception.
      void resolve(std::exception_ptr e);

    private:
      template<typename T1, typename T2>
      friend struct std::pair;
      Beam::Routines::Eval<Type> m_eval;

      QtFuture(Beam::Routines::Eval<Type> eval);
  };

  //! Returns a pair consisting of a QtFuture<T> and its corresponding
  //! QtPromise<T>.
  template<typename T>
  std::pair<QtFuture<T>, QtPromise<T>> make_future() {
    auto async = std::make_unique<Beam::Routines::Async<T>>();
    auto eval = async->GetEval();
    return std::pair<QtFuture<T>, QtPromise<T>>(std::piecewise_construct,
      std::forward_as_tuple(std::move(eval)),
      std::forward_as_tuple([async = std::move(async)] {
        return async->Get();
      }, LaunchPolicy::ASYNC));
  }

  template<typename T>
  void QtFuture<T>::resolve(Type value) {
    m_eval.SetResult(std::move(value));
  }

  template<typename T>
  void QtFuture<T>::resolve(std::exception_ptr e) {
    m_eval.SetException(std::move(e));
  }

  template<typename T>
  QtFuture<T>::QtFuture(Beam::Routines::Eval<Type> eval)
    : m_eval(std::move(eval)) {}
}

#endif
