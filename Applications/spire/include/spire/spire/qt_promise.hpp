#ifndef SPIRE_QT_PROMISE_HPP
#define SPIRE_QT_PROMISE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/noncopyable.hpp>
#include "spire/spire/qt_promise_imp.hpp"
#include "spire/spire/spire.hpp"

namespace spire {

  /*! \brief Performs an asyncronous computation and signals the result within a
             Qt-thread in a thread-safe manner.
      \tparam T The type of value to compute.
  */
  template<typename T>
  class qt_promise : private boost::noncopyable {
    public:

      //! The type of value to compute.
      using type = T;

      //! Constructs an empty promise.
      qt_promise() = default;

      //! Constructs a Qt promise.
      /*!
        \param executor The callable performing the computation.
      */
      template<typename Executor>
      qt_promise(Executor&& executor);

      qt_promise(qt_promise&& other);

      ~qt_promise();

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
      qt_promise& operator =(qt_promise&& other);

    private:
      std::shared_ptr<details::base_qt_promise_imp<type>> m_imp;
  };

  //! Makes a Qt promise.
  /*!
    \param executor The callable performing the computation.
  */
  template<typename Executor>
  auto make_qt_promise(Executor&& executor) {
    return qt_promise<std::result_of_t<Executor()>>(
      std::forward<Executor>(executor));
  }

  template<typename T>
  template<typename Executor>
  qt_promise<T>::qt_promise(Executor&& executor) {
    auto imp = std::make_shared<details::qt_promise_imp<Executor>>(
      std::forward<Executor>(executor));
    imp->bind(imp);
    m_imp = std::move(imp);
  }

  template<typename T>
  qt_promise<T>::qt_promise(qt_promise&& other)
      : m_imp(std::move(other.m_imp)) {}

  template<typename T>
  qt_promise<T>::~qt_promise() {
    disconnect();
  }

  template<typename T>
  template<typename F>
  void qt_promise<T>::then(F&& continuation) {
    m_imp->then(std::forward<F>(continuation));
  }

  template<typename T>
  void qt_promise<T>::disconnect() {
    if(m_imp == nullptr) {
      return;
    }
    m_imp->disconnect();
    m_imp = nullptr;
  }

  template<typename T>
  qt_promise<T>& qt_promise<T>::operator =(qt_promise&& other) {
    disconnect();
    m_imp = std::move(other.m_imp);
    return *this;
  }
}

#endif
