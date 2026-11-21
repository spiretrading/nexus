#ifndef SPIRE_BLOTTER_TASK_VIEW_HPP
#define SPIRE_BLOTTER_TASK_VIEW_HPP
#include "Spire/Blotter/BlotterTaskEntry.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
  class CanvasTableWidget;

  /** Displays task details for a blotter. */
  class BlotterTaskView : public QWidget {
    public:

      /**
       * Constructs a BlotterTaskView.
       * @param tasks The blotter's tasks.
       * @param current The current index in source table coordinates.
       * @param parent The parent widget.
       */
      BlotterTaskView(std::shared_ptr<TasksModel> tasks,
        std::shared_ptr<TableView::CurrentModel> current,
        QWidget* parent = nullptr);

    private:
      std::shared_ptr<TasksModel> m_tasks;
      std::shared_ptr<TableView::CurrentModel> m_current;
      std::shared_ptr<Task> m_task;
      CanvasTableWidget* m_canvas;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_task_connection;

      void on_current(const boost::optional<TableIndex>& current);
      void on_task(const TasksModel::Operation& operation);
  };
}

#endif
