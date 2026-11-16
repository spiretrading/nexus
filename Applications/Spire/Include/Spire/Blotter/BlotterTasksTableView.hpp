#ifndef SPIRE_BLOTTER_TASKS_TABLE_VIEW_HPP
#define SPIRE_BLOTTER_TASKS_TABLE_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/BlotterTaskEntry.hpp"

namespace Spire {

  /** Displays a table of a blotter's order execution tasks. */
  class BlotterTasksTableView : public QWidget {
    public:

      /**
       * Constructs a BlotterTasksTableView.
       * @param tasks The blotter's order execution task entries.
       * @param parent The parent widget.
       */
      explicit BlotterTasksTableView(
        std::shared_ptr<TasksModel> tasks, QWidget* parent = nullptr);
  };
}

#endif
