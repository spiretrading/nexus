#ifndef SPIRE_BLOTTER_TASKS_TABLE_VIEW_HPP
#define SPIRE_BLOTTER_TASKS_TABLE_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a table of a blotter's order execution tasks. */
  class BlotterTasksTableView : public QWidget {
    public:

      /**
       * Constructs an empty BlotterTasksTableView.
       * @param parent The parent widget.
       */
      explicit BlotterTasksTableView(QWidget* parent = nullptr);
  };
}

#endif
