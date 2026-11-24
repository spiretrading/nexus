#ifndef SPIRE_EXECUTOR_HPP
#define SPIRE_EXECUTOR_HPP
#include <memory>
#include <Aspen/Box.hpp>
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
      friend class Task;
      struct State;
      std::shared_ptr<State> m_state;

      Executor(const Executor&) = delete;
      Executor& operator =(const Executor&) = delete;
      void Open(std::shared_ptr<void> lifetime);
  };
}

#endif
