#ifndef SPIRE_BLOTTER_TASK_VIEW_HPP
#define SPIRE_BLOTTER_TASK_VIEW_HPP
#include <vector>
#include <QWidget>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's tasks. */
  class BlotterTaskView : public QWidget {
    public:

      /**
       * Signals to execute a list of selected Tasks.
       * @param tasks The list of Tasks to execute.
       */
      using ExecuteSignal =
        Signal<void (const std::vector<std::shared_ptr<Task>>& tasks)>;

      /**
       * Signals to cancel a list of selected Tasks.
       * @param tasks The list of Tasks to cancel.
       */
      using CancelSignal =
        Signal<void (const std::vector<std::shared_ptr<Task>>& tasks)>;

      /**
       * Constructs a BlotterTaskView.
       * @param is_active Whether this blotter is the active blotter.
       * @param is_pinned Whether this blotter is pinned.
       * @param tasks The list of tasks belonging to this blotter.
       * @param parent The parent widget.
       */
      BlotterTaskView(std::shared_ptr<BooleanModel> is_active,
        std::shared_ptr<BooleanModel> is_pinned,
        std::shared_ptr<TaskListModel> tasks, QWidget* parent = nullptr);

      /** Whether this is the application's active blotter. */
      const std::shared_ptr<BooleanModel>& is_active();

      /** Whether this blotter persists after being closed. */
      const std::shared_ptr<BooleanModel>& is_pinned();

      /** Returns this blotter's tasks. */
      const std::shared_ptr<TaskListModel>& get_tasks();

      /** Connects a slot to the ExecuteSignal. */
      boost::signals2::connection connect_execute_signal(
        const ExecuteSignal::slot_type& slot) const;

      /** Connects a slot to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    private:
      mutable ExecuteSignal m_execute_signal;
      mutable CancelSignal m_cancel_signal;
  };
}

#endif
