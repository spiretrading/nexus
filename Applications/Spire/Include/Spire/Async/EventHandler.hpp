#ifndef SPIRE_EVENT_HANDLER_HPP
#define SPIRE_EVENT_HANDLER_HPP
#include <exception>
#include <functional>
#include <memory>
#include <utility>
#include <Beam/Queues/TaskQueue.hpp>
#include <QTimer>
#include "Spire/Async/Async.hpp"

namespace Spire {

  /**
   * Allows for handling events pushed onto a Queue in a manner synchronized
   * with the main Qt event-loop.
   */
  class EventHandler : public QObject {
    public:

      /** Constructs an EventHandler. */
      EventHandler();

      /**
       * Returns a QueueWriter used to call a slot on every push.
       * @param slot The slot to call when a new value is pushed.
       * @return A QueueWriter that translates a push into a slot invocation.
       */
      template<typename T, typename F>
      auto get_slot(F&& slot);

      /**
       * Returns a QueueWriter used to call a slot on every push.
       * @param slot The slot to call when a new value is pushed.
       * @param break_slot The slot to call when this Queue is broken.
       * @return A QueueWriter that translates a push into a slot invocation.
       */
      template<typename T, typename F, typename B>
      auto get_slot(F&& slot, B&& break_slot);

      /**
       * Pushes a task onto this event handler to be executed within the main
       * Qt event-loop.
       * @param f The task to push.
       */
      template<typename F>
      void push(F&& f);

    private:
      QTimer m_update_timer;
      std::shared_ptr<Beam::TaskQueue> m_tasks;

      EventHandler(const EventHandler&) = delete;
      EventHandler& operator =(const EventHandler&) = delete;
      void on_expired();
  };

  template<typename T, typename F>
  auto EventHandler::get_slot(F&& slot) {
    return m_tasks->GetSlot<T>(std::forward<F>(slot));
  }

  template<typename T, typename F, typename B>
  auto EventHandler::get_slot(F&& slot, B&& break_slot) {
    return m_tasks->GetSlot<T>(std::forward<F>(slot),
      std::forward<B>(break_slot));
  }

  template<typename F>
  void EventHandler::push(F&& f) {
    m_tasks->Push(std::forward<F>(f));
  }
}

#endif
