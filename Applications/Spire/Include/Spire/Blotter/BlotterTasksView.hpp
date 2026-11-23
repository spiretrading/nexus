#ifndef SPIRE_BLOTTER_TASKS_VIEW_HPP
#define SPIRE_BLOTTER_TASKS_VIEW_HPP
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
  class BlotterTasksTableView;

  /** Displays a blotter's order execution tasks. */
  class BlotterTasksView : public QWidget {
    public:

      /**
       * Signals a request to run the selected tasks.
       * @param tasks The tasks selected when the run was requested.
       */
      using RunSignal =
        Signal<void (const std::vector<std::shared_ptr<Task>>& tasks)>;

      /**
       * Signals a request to cancel the selected tasks.
       * @param tasks The tasks selected when cancellation was requested.
       */
      using CancelSignal =
        Signal<void (const std::vector<std::shared_ptr<Task>>& tasks)>;

      /**
       * Signals a key-bound request to cancel tasks.
       * @param operation The requested cancel operation.
       */
      using CancelOperationSignal =
        Signal<void (CancelKeyBindingsModel::Operation operation)>;

      /**
       * Constructs a BlotterTasksView.
       * @param model The blotter whose tasks to display.
       * @param bindings The user's cancel key bindings.
       * @param parent The parent widget.
       */
      BlotterTasksView(std::shared_ptr<BlotterModel> model,
        std::shared_ptr<CancelKeyBindingsModel> bindings,
        QWidget* parent = nullptr);

      /** Returns the current index in source table coordinates. */
      const std::shared_ptr<TableView::CurrentModel>& get_current() const;

      /** Connects a slot to the RunSignal. */
      boost::signals2::connection connect_run_signal(
        const RunSignal::slot_type& slot) const;

      /** Connects a slot to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

      /** Connects a slot to the CancelOperationSignal. */
      boost::signals2::connection connect_cancel_operation_signal(
        const CancelOperationSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      friend class BlotterWindowSettings;
      mutable RunSignal m_run_signal;
      mutable CancelSignal m_cancel_signal;
      mutable CancelOperationSignal m_cancel_operation_signal;
      std::shared_ptr<CancelKeyBindingsModel> m_bindings;
      std::shared_ptr<TasksModel> m_tasks;
      std::shared_ptr<TableSelectionModel> m_selection;
      BlotterTasksTableView* m_table;

      std::vector<std::shared_ptr<Task>> get_selected_tasks() const;
      void on_run();
      void on_cancel();
  };
}

#endif
