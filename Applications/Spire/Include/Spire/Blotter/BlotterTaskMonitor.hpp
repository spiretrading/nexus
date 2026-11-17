#ifndef SPIRE_BLOTTER_TASK_MONITOR_HPP
#define SPIRE_BLOTTER_TASK_MONITOR_HPP
#include <QString>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** Defines a named expression to observe on each task in a blotter. */
  struct BlotterTaskMonitor {

    /** The name of the monitor column. */
    QString m_name;

    /** The expression evaluated for each task, or null for an empty column. */
    std::shared_ptr<const CanvasNode> m_expression;
  };

  /** The ordered monitor definitions for a blotter's tasks. */
  using TaskMonitorsModel = ListModel<BlotterTaskMonitor>;

  /** Returns a new list of the default task monitors. */
  std::shared_ptr<TaskMonitorsModel> make_default_task_monitors();
}

#endif
