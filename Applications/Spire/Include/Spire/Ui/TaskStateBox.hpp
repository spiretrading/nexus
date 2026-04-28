#ifndef SPIRE_TASK_STATE_BOX_HPP
#define SPIRE_TASK_STATE_BOX_HPP
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

  /** A ValueModel over a Task::State. */
  using TaskStateModel = ValueModel<Task::State>;

  /** A LocalValueModel over a Task::State. */
  using LocalTaskStateModel = LocalValueModel<Task::State>;

  /** An EnumBox specialized for a Task::State. */
  using TaskStateBox = EnumBox<Task::State>;

  /**
   * Returns a new TaskStateBox using a LocalTaskStateModel.
   * @param parent The parent widget.
   */
  TaskStateBox* make_task_state_box(QWidget* parent = nullptr);

  /**
   * Returns a new TaskStateBox using a LocalTaskStateModel and
   * initial current value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  TaskStateBox* make_task_state_box(
    Task::State current, QWidget* parent = nullptr);

  /**
   * Returns a TaskStateBox.
   * @param current The current value model.
   * @param parent The parent widget.
   */
  TaskStateBox* make_task_state_box(
    std::shared_ptr<TaskStateModel> current, QWidget* parent = nullptr);
}

#endif
