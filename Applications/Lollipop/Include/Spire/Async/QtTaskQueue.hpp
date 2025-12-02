#ifndef SPIRE_QT_TASK_QUEUE_HPP
#define SPIRE_QT_TASK_QUEUE_HPP
#include <atomic>
#include <exception>
#include <functional>
#include <Beam/Queues/CallbackQueue.hpp>

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

      /**
       * Adds a value to the end of the Queue.
       * @param value The value to add to the end of the Queue.
       */
      void push(const Target& value) override;

      /**
       * Adds a value to the end of the Queue.
       * @param value The value to add to the end of the Queue.
       */
      void push(Target&& value) override;

      /**
       * Breaks this Queue, indicating no further values will be published.
       * @param exception The reason why the Queue was broken.
       */
      void close(const std::exception_ptr& exception) override;

      using QueueWriter<std::function<void ()>>::close;
    private:
      struct EventHandler;
      std::atomic_bool m_is_broken;
      std::exception_ptr m_break_exception;
      std::unique_ptr<EventHandler> m_event_handler;
      Beam::CallbackQueue m_callbacks;

      void safe_push(Target&& value);
      template<typename T, typename F, typename B>
      auto get_slot_helper(F&& callback, B&& breakCallback);
  };

  template<typename T, typename F>
  auto QtTaskQueue::get_slot(F&& callback) {
    return get_slot<T>(std::forward<F>(callback), [] (const auto&) {});
  }

  template<typename T>
  auto QtTaskQueue::get_slot(const std::function<void (const T&)>& callback) {
    return get_slot<T>(callback, [] (const auto&) {});
  }

  template<typename T, typename F, typename B>
  auto QtTaskQueue::get_slot(F&& callback, B&& break_callback) {
    return get_slot_helper<T>(
      std::forward<F>(callback), std::forward<B>(break_callback));
  }

  template<typename T>
  auto QtTaskQueue::get_slot(const std::function<void (const T&)>& callback,
      const std::function<void (const std::exception_ptr&)>& break_callback) {
    return get_slot_helper<T>(callback, break_callback);
  }

  template<typename T, typename F, typename B>
  auto QtTaskQueue::get_slot_helper(F&& callback, B&& break_callback) {
    return m_callbacks.get_slot<T>(
      [this, callback = std::make_shared<std::remove_reference_t<F>>(
          std::forward<F>(callback))] (const T& value) {
        safe_push([=] {
          (*callback)(value);
        });
      },
      [this, break_callback = std::make_shared<std::remove_reference_t<B>>(
          std::forward<B>(break_callback))] (const std::exception_ptr& e) {
        safe_push([=] {
          (*break_callback)(e);
        });
      });
  }
}

#endif
