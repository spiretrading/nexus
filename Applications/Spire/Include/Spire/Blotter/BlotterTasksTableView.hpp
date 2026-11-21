#ifndef SPIRE_BLOTTER_TASKS_TABLE_VIEW_HPP
#define SPIRE_BLOTTER_TASKS_TABLE_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/BlotterTaskEntry.hpp"
#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Displays a table of a blotter's order execution tasks. */
  class BlotterTasksTableView : public QWidget {
    public:

      /**
       * Constructs a BlotterTasksTableView.
       * @param tasks The blotter's order execution task entries.
       * @param monitors The monitors to display after the fixed task columns.
       * @param selection The selection in source table coordinates.
       * @param parent The parent widget.
       */
      BlotterTasksTableView(std::shared_ptr<TasksModel> tasks,
        std::shared_ptr<TaskMonitorsModel> monitors,
        std::shared_ptr<TableSelectionModel> selection,
        QWidget* parent = nullptr);

      /** Returns the current index in source table coordinates. */
      const std::shared_ptr<TableView::CurrentModel>& get_current() const;

    private:
      std::shared_ptr<TasksModel> m_tasks;
      std::shared_ptr<TaskMonitorsModel> m_monitors;
      std::shared_ptr<TableSelectionModel> m_selection;
      std::shared_ptr<TableView::CurrentModel> m_current;
      TableView* m_table_view;
      boost::signals2::scoped_connection m_monitor_connection;

      TableView* make_table() const;
      void on_monitor(const TaskMonitorsModel::Operation& operation);
  };
}

#endif
