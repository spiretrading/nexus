#ifndef SPIRE_QT_PROMISE_HPP
#define SPIRE_QT_PROMISE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/noncopyable.hpp>
#include "spire/spire/qt_promise_imp.hpp"
#include "spire/spire/spire.hpp"

namespace spire {

  //! Performs an asyncronous computation and signals the result within a
  //! Qt-thread in a thread-safe manner.
  class qt_promise : private boost::noncopyable {
    public:

      //! Constructs an empty promise.
      qt_promise() = default;

      //! Constructs a Qt promise.
      /*!
        \param executor The callable performing the computation.
        \param callback The callable receiving the completion signal.
      */
      template<typename Executor, typename Callback>
      qt_promise(Executor&& executor, Callback&& callback);

      qt_promise(qt_promise&& other);

      ~qt_promise();

      //! Disconnects from this promise, upon disconnection the callback
      //! will not be called when the executor completes its computation.
      void disconnect();

      //! Disconnects this promise and then moves another promise into this.
      qt_promise& operator =(qt_promise&& other);

    private:
      std::shared_ptr<details::base_qt_promise_imp> m_imp;
  };

  template<typename Executor, typename Callback>
  qt_promise::qt_promise(Executor&& executor, Callback&& callback) {
    auto imp = std::make_shared<details::qt_promise_imp<std::decay_t<Executor>,
      std::decay_t<Callback>>>(std::forward<Executor>(executor),
      std::forward<Callback>(callback));
    imp->bind(imp);
    m_imp = std::move(imp);
  }
}

#endif
