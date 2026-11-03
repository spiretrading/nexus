#ifndef SPIRE_EXECUTOR_HPP
#define SPIRE_EXECUTOR_HPP
#include <condition_variable>
#include <mutex>
#include <Aspen/Aspen.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
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
      Aspen::CommitFlag m_flag;
      Beam::RoutineHandler m_reactorLoop;
      bool m_has_update;
      Aspen::Shared<Aspen::Queue<Aspen::SharedBox<void>>> m_producer;
      Aspen::Concur<Aspen::Shared<Aspen::Queue<Aspen::SharedBox<void>>>>
        m_reactor;
      std::condition_variable m_updateCondition;
      Beam::OpenState m_openState;

      Aspen::State Commit(int sequence);
      void RunLoop();
      void OnUpdate();
  };
}

#endif
