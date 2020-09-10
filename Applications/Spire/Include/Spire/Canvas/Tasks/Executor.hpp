#ifndef SPIRE_EXECUTOR_HPP
#define SPIRE_EXECUTOR_HPP
#include <mutex>
#include <Aspen/Aspen.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /** Executes multiple Aspen reactors from within a Beam routine. */
  class Executor {
    public:

      /** Constructs an Executor. */
      Executor();

      ~Executor();

      /**
       * Adds a Reactor to execute.
       * @param reactor The reactor to execute.
       */
      void Add(Aspen::Box<void> reactor);

      void Open();

      void Close();

    private:
      std::mutex m_mutex;
      Aspen::Trigger m_trigger;
      Beam::Routines::RoutineHandler m_reactorLoop;
      bool m_has_update;
      Aspen::Shared<Aspen::Queue<Aspen::SharedBox<void>>> m_producer;
      Aspen::Concur<Aspen::Shared<Aspen::Queue<Aspen::SharedBox<void>>>>
        m_reactor;
      Beam::Threading::ConditionVariable m_updateCondition;
      Beam::IO::OpenState m_openState;

      void RunLoop();
      void OnUpdate();
  };
}

#endif
