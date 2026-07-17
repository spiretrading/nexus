#ifndef NEXUS_SIMULATION_EXECUTION_REPORT_QUEUE_HPP
#define NEXUS_SIMULATION_EXECUTION_REPORT_QUEUE_HPP
#include <deque>
#include <functional>
#include <mutex>
#include <Beam/Routines/RoutineHandler.hpp>

namespace Nexus {

  /**
   * Queues the publication of simulated ExecutionReports so that they can be
   * published one at a time from a single Routine.
   */
  class SimulationExecutionReportQueue {
    public:

      /**
       * Sets the callback invoked whenever a publication is queued, used to
       * wake up whoever flushes this queue.
       * @param slot The callback to invoke.
       */
      void set_slot(std::function<void ()> slot);

      /**
       * Queues a report publication.
       * @param publication The publication to queue.
       */
      void push(std::function<void ()> publication);

      /**
       * Performs the next queued publication and then flushes all pending
       * Routines.
       * @return <code>true</code> iff a publication was performed.
       */
      bool flush_next();

      /**
       * Performs every queued publication, including any queued as a
       * consequence of a publication being performed.
       */
      void flush();

    private:
      std::mutex m_mutex;
      std::function<void ()> m_slot;
      std::deque<std::function<void ()>> m_publications;
  };

  inline void SimulationExecutionReportQueue::set_slot(
      std::function<void ()> slot) {
    auto lock = std::lock_guard(m_mutex);
    m_slot = std::move(slot);
  }

  inline void SimulationExecutionReportQueue::push(
      std::function<void ()> publication) {
    auto slot = [&] {
      auto lock = std::lock_guard(m_mutex);
      m_publications.push_back(std::move(publication));
      return m_slot;
    }();
    if(slot) {
      slot();
    }
  }

  inline bool SimulationExecutionReportQueue::flush_next() {
    auto publication = std::function<void ()>();
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_publications.empty()) {
        return false;
      }
      publication = std::move(m_publications.front());
      m_publications.pop_front();
    }
    publication();
    Beam::flush_pending_routines();
    return true;
  }

  inline void SimulationExecutionReportQueue::flush() {
    while(flush_next()) {}
  }
}

#endif
