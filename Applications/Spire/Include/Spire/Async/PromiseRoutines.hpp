#ifndef SPIRE_PROMISE_ROUTINES_HPP
#define SPIRE_PROMISE_ROUTINES_HPP
#include <concepts>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>
#include <Beam/Routines/Scheduler.hpp>
#include <Beam/Utilities/Singleton.hpp>

namespace Spire {

  /** Owns the routines spawned by asynchronously launched QtPromises. */
  class PromiseRoutines : public Beam::Singleton<PromiseRoutines> {
    public:

      /** Constructs a PromiseRoutines owning no routines. */
      PromiseRoutines();

      ~PromiseRoutines();

      /**
       * Spawns a routine owned by this object.
       * @param f The callable to invoke within the routine.
       */
      template<std::invocable F>
      void spawn(F&& f);

      /** Blocks until every routine owned by this object has completed. */
      void wait();

    private:
      std::mutex m_mutex;
      std::condition_variable m_condition;
      int m_count;

      void release();
  };

  inline PromiseRoutines::PromiseRoutines()
      : m_count(0) {
    Beam::Details::Scheduler::get();
  }

  inline PromiseRoutines::~PromiseRoutines() {
    wait();
  }

  template<std::invocable F>
  void PromiseRoutines::spawn(F&& f) {
    {
      auto lock = std::lock_guard(m_mutex);
      ++m_count;
    }
    auto guard = std::shared_ptr<void>(nullptr, [this] (auto) {
      release();
    });
    Beam::spawn(
      [f = std::forward<F>(f), guard = std::move(guard)] () mutable {
        f();
      });
  }

  inline void PromiseRoutines::wait() {
    auto lock = std::unique_lock(m_mutex);
    m_condition.wait(lock, [&] {
      return m_count == 0;
    });
  }

  inline void PromiseRoutines::release() {
    auto lock = std::lock_guard(m_mutex);
    --m_count;
    if(m_count == 0) {
      m_condition.notify_all();
    }
  }
}

#endif
