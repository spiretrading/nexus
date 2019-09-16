#ifndef SPIRE_REACTOR_MONITOR_HPP
#define SPIRE_REACTOR_MONITOR_HPP
#include <Aspen/Aspen.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /** Used to synchronously monitor multiple Reactors. */
  class ReactorMonitor {
    public:

      /** Constructs a ReactorMonitor. */
      ReactorMonitor();

      /**
       * Adds a Reactor to monitor.
       * @param reactor The reactor to monitor.
       */
      void Add(Aspen::Box<void> reactor);

      /**
       * Invokes a function from within a reactor.
       * @param f The callback to invoke.
       */
      template<typename F>
      void Do(F&& f);

      /** Waits for all Reactors to complete. */
      void Wait();

      void Open();

      void Close();
  };
}

#endif
