#ifndef SPIRE_BLOTTER_TASK_ENTRY_HPP
#define SPIRE_BLOTTER_TASK_ENTRY_HPP
#include <vector>
#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** Stores a task's state and monitors within a blotter. */
  struct BlotterTaskEntry {

    /** The task represented by this entry. */
    std::shared_ptr<Task> m_task;

    /**
     * Whether the task remains in the blotter after reaching a terminal state.
     */
    bool m_is_pinned = false;

    /** The task's latest execution state. */
    Task::State m_state = Task::State::READY;

    /** The task's observers, in monitor column order. */
    std::vector<std::shared_ptr<CanvasObserver>> m_monitors;
  };

  /** The observable collection of a blotter's task entries. */
  using TasksModel = ListModel<BlotterTaskEntry>;
}

#endif
