#ifndef SPIRE_BLOTTER_TASKS_COMMAND_BAR_HPP
#define SPIRE_BLOTTER_TASKS_COMMAND_BAR_HPP
#include <QWidget>

namespace Spire {

  /** Displays commands for a blotter and its order execution tasks. */
  class BlotterTasksCommandBar : public QWidget {
    public:

      /**
       * Constructs an empty BlotterTasksCommandBar.
       * @param parent The parent widget.
       */
      explicit BlotterTasksCommandBar(QWidget* parent = nullptr);
  };
}

#endif
