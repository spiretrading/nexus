#ifndef SPIRE_BLOTTER_TASKS_VIEW_HPP
#define SPIRE_BLOTTER_TASKS_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a blotter's order execution tasks. */
  class BlotterTasksView : public QWidget {
    public:

      /**
       * Constructs an empty BlotterTasksView.
       * @param parent The parent widget.
       */
      explicit BlotterTasksView(QWidget* parent = nullptr);
  };
}

#endif
