#ifndef SPIRE_BLOTTER_TASK_MONITOR_HPP
#define SPIRE_BLOTTER_TASK_MONITOR_HPP
#include <memory>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUniquePtr.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Blotter/Blotter.hpp"

namespace Spire {

  /** Stores a single entry used to monitor a Task from a BlotterWindow. */
  class BlotterTaskMonitor {
    public:

      /** Constructs an empty BlotterTaskModel. */
      BlotterTaskMonitor();

      /**
       * Constructs a BlotterTaskMonitor.
       * @param name The name of the monitor.
       * @param monitor The monitor to attach to a Task.
       */
      BlotterTaskMonitor(const std::string& name, const CanvasNode& monitor);

      /**
       * Copies a BlotterTaskMonitor.
       * @param monitor The BlotterTaskMonitor to copy.
       */
      BlotterTaskMonitor(const BlotterTaskMonitor& monitor);

      ~BlotterTaskMonitor() = default;

      /**
       * Assigns a BlotterTaskMonitor.
       * @param monitor The BlotterTaskMonitor to assign from.
       * @return A reference to <i>*this</i>.
       */
      BlotterTaskMonitor& operator =(const BlotterTaskMonitor& monitor);

      /** Returns the name of this monitor. */
      const std::string& GetName() const;

      /** Returns the monitor to attach to a Task. */
      const CanvasNode& GetMonitor() const;

    private:
      friend struct Beam::DataShuttle;
      std::string m_name;
      std::unique_ptr<CanvasNode> m_monitor;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BlotterTaskMonitor::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("monitor", m_monitor);
  }
}

#endif
