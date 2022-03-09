#ifndef SPIRE_QT_TASK_QUEUE_HPP
#define SPIRE_QT_TASK_QUEUE_HPP
#include <Beam/Queues/TaskQueue.hpp>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Runs tasks pushed to it within Qt's event loop. */
  class QtTaskQueue : public Beam::QueueWriter<std::function<void ()>> {
    public:

      /** The type being pushed. */
      using Target = Beam::QueueWriter<std::function<void ()>>::Target;

      /** Constructs a QtTaskQueue. */
      QtTaskQueue();

      ~QtTaskQueue();

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, typename F>
      auto get_slot(F&& callback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @return A queue that translates a push into a callback.
       */
      template<typename T>
      auto get_slot(const std::function<void (const T&)>& callback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @param break_callback The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, typename F, typename B>
      auto get_slot(F&& callback, B&& break_callback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @param break_callback The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T>
      auto get_slot(const std::function<void (const T&)>& callback,
        const std::function<void (const std::exception_ptr&)>& break_callback);

      /** Waits for this queue to be broken and all tasks to complete. */
      void wait();

      /**
       * Adds a value to the end of the Queue.
       * @param value The value to add to the end of the Queue.
       */
      void push(const Target& value);

      /**
       * Adds a value to the end of the Queue.
       * @param value The value to add to the end of the Queue.
       */
      void push(Target&& value);

      /** Breaks this Queue, indicating no further values will be published.  */
      void close();

      /**
       * Breaks this Queue, indicating no further values will be published.
       * @param exception The reason why the Queue was broken.
       */
      void close(const std::exception_ptr& exception);

      using QueueWriter<std::function<void ()>>::Break;

    private:
      struct EventHandler;
      std::unique_ptr<EventHandler> m_event_handler;
      Beam::TaskQueue m_tasks;

      void Push(const Target& value) override;
      void Push(Target&& value) override;
      void Break(const std::exception_ptr& exception) override;
  };

  template<typename T, typename F>
  auto QtTaskQueue::get_slot(F&& callback) {
    return m_tasks.GetSlot(std::forward<F>(callback));
  }

  template<typename T>
  auto QtTaskQueue::get_slot(const std::function<void (const T&)>& callback) {
    return m_tasks.GetSlot(callback);
  }

  template<typename T, typename F, typename B>
  auto QtTaskQueue::get_slot(F&& callback, B&& break_callback) {
    return m_tasks.GetSlot(
      std::forward<F>(callback), std::forward<B>(break_callback));
  }

  template<typename T>
  auto QtTaskQueue::get_slot(const std::function<void (const T&)>& callback,
      const std::function<void (const std::exception_ptr&)>& break_callback) {
    return m_tasks.GetSlot(callback, break_callback);
  }
}

#endif
