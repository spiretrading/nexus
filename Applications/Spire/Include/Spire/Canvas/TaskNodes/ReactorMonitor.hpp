#ifndef SPIRE_REACTOR_MONITOR_HPP
#define SPIRE_REACTOR_MONITOR_HPP
#include <mutex>
#include <Aspen/Aspen.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <boost/noncopyable.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /** Used to synchronously monitor multiple Reactors. */
  class ReactorMonitor : private boost::noncopyable {
    public:

      /** Constructs a ReactorMonitor. */
      ReactorMonitor();

      ~ReactorMonitor();

      /**
       * Adds a Reactor to monitor.
       * @param reactor The reactor to monitor.
       */
      void Add(Aspen::Box<void> reactor);

      void Open();

      void Close();

    private:
      std::mutex m_mutex;
      Beam::Routines::RoutineHandler m_reactorLoop;
      bool m_has_update;
      Aspen::Shared<Aspen::Queue<Aspen::Box<void>>> m_producer;
      Aspen::Group<Aspen::Shared<Aspen::Queue<Aspen::Box<void>>>> m_reactor;
      Beam::Threading::ConditionVariable m_updateCondition;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void RunLoop();
      void OnUpdate();
  };
}

#endif
