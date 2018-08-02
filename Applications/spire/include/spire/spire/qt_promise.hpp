#ifndef SPIRE_QT_PROMISE_HPP
#define SPIRE_QT_PROMISE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/noncopyable.hpp>
#include "spire/spire/qt_promise_imp.hpp"
#include "spire/spire/spire.hpp"

namespace Spire {

  /*! \brief Performs an asyncronous computation and signals the result within a
             Qt-thread in a thread-safe manner.
      \tparam T The type of value to compute.
  */
  template<typename T>
  class QtPromise : private boost::noncopyable {
    public:

      //! The type of value to compute.
      using Type = T;

      //! Constructs an empty promise.
      QtPromise() = default;

      //! Constructs a Qt promise.
      /*!
        \param executor The callable performing the computation.
      */
      template<typename Executor>
      QtPromise(Executor&& executor);

      QtPromise(QtPromise&& other);

      ~QtPromise();

      //! Assigns a function to be called when the computation completes.
      /*!
        \param continuation The function to call when the computation completes.
      */
      template<typename F>
      void then(F&& continuation);

      //! Disconnects from this promise, upon disconnection the callback
      //! will not be called when the executor completes its computation.
      void disconnect();

      //! Disconnects this promise and then moves another promise into this.
      QtPromise& operator =(QtPromise&& other);

    private:
      std::shared_ptr<details::BaseQtPromiseImp<Type>> m_imp;
  };

  //! Makes a Qt promise.
  /*!
    \param executor The callable performing the computation.
  */
  template<typename Executor>
  auto make_qt_promise(Executor&& executor) {
    return QtPromise<std::result_of_t<Executor()>>(
      std::forward<Executor>(executor));
  }

  template<typename T>
  template<typename Executor>
  QtPromise<T>::QtPromise(Executor&& executor) {
    auto imp = std::make_shared<details::qt_promise_imp<Executor>>(
      std::forward<Executor>(executor));
    imp->bind(imp);
    m_imp = std::move(imp);
  }

  template<typename T>
  QtPromise<T>::QtPromise(QtPromise&& other)
      : m_imp(std::move(other.m_imp)) {}

  template<typename T>
  QtPromise<T>::~QtPromise() {
    disconnect();
  }

  template<typename T>
  template<typename F>
  void QtPromise<T>::then(F&& continuation) {
    m_imp->then(std::forward<F>(continuation));
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
}

#endif
