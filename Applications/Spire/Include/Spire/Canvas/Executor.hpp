#ifndef SPIRE_EXECUTOR_HPP
#define SPIRE_EXECUTOR_HPP
#include <mutex>
#include <Aspen/Aspen.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /** Executes an Aspen reactor from within a Beam routine. */
  class Executor {
    public:

      /**
       * Constructs an Executor.
       * @param reactor The reactor to execute.
       */
      explicit Executor(Aspen::Box<void> reactor);

      ~Executor();

    private:
      std::mutex m_mutex;
      Aspen::Trigger m_trigger;
      Beam::Routines::RoutineHandler m_reactor_loop;
      bool m_has_update;
      Aspen::Box<void> m_reactor;
      Beam::Threading::ConditionVariable m_update_condition;
      Beam::IO::OpenState m_open_state;

      void run();
      void on_update();
  };
}

#endif
