#ifndef SPIRE_BLOTTER_TASKS_VIEW_HPP
#define SPIRE_BLOTTER_TASKS_VIEW_HPP
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Ui/TableSelectionModel.hpp"

namespace Spire {

  /** Displays a blotter's order execution tasks. */
  class BlotterTasksView : public QWidget {
    public:

      /**
       * Signals a request to cancel the selected tasks.
       * @param tasks The tasks selected when cancellation was requested.
       */
      using CancelTasksSignal =
        Signal<void (const std::vector<std::shared_ptr<Task>>& tasks)>;

      /**
       * Constructs a BlotterTasksView.
       * @param model The blotter whose tasks to display.
       * @param parent The parent widget.
       */
      explicit BlotterTasksView(
        std::shared_ptr<BlotterModel> model, QWidget* parent = nullptr);

      /** Connects a slot to the CancelTasksSignal. */
      boost::signals2::connection connect_cancel_tasks_signal(
        const CancelTasksSignal::slot_type& slot) const;

    private:
      mutable CancelTasksSignal m_cancel_tasks_signal;
      std::shared_ptr<TasksModel> m_tasks;
      std::shared_ptr<TableSelectionModel> m_selection;

      void on_cancel();
  };
}

#endif
