#ifndef SPIRE_EVENT_HANDLER_HPP
#define SPIRE_EVENT_HANDLER_HPP
#include <exception>
#include <functional>
#include <memory>
#include <Beam/Queues/TaskQueue.hpp>
#include <boost/noncopyable.hpp>
#include <QTimer>
#include "spire/spire/spire.hpp"

namespace Spire {

  /** Allows for handling events pushed onto a Queue in a manner synchronized
      with the main Qt event-loop. */
  class EventHandler : public QObject, private boost::noncopyable {
    public:

      //! Constructs an EventHandler.
      EventHandler();

      //! Returns a Queue used to call a slot on every push.
      /*!
        \param slot The slot to call when a new value is pushed.
        \return A Queue that translates a push into a slot invocation.
      */
      template<typename T>
      std::shared_ptr<Beam::CallbackWriterQueue<T>> get_slot(
        const std::function<void (const T& value)>& slot);

      //! Returns a Queue used to call a slot on every push.
      /*!
        \param slot The slot to call when a new value is pushed.
        \param break_slot The slot to call when this Queue is broken.
        \return A Queue that translates a push into a slot invocation.
      */
      template<typename T>
      std::shared_ptr<Beam::CallbackWriterQueue<T>> get_slot(
        const std::function<void (const T& value)>& slot,
        const std::function<void (const std::exception_ptr& e)>& break_slot);

    private:
      QTimer m_update_timer;
      std::shared_ptr<Beam::TaskQueue> m_tasks;

      void on_expired();
  };

  template<typename T>
  std::shared_ptr<Beam::CallbackWriterQueue<T>> EventHandler::get_slot(
      const std::function<void (const T& value)>& slot) {
    return m_tasks->GetSlot(slot);
  }

  template<typename T>
  std::shared_ptr<Beam::CallbackWriterQueue<T>> EventHandler::get_slot(
      const std::function<void (const T& value)>& slot,
      const std::function<void (const std::exception_ptr& e)>& break_slot) {
    return m_tasks->GetSlot(slot, break_slot);
  }
}

#endif
