#ifndef SPIRE_BLOTTER_TASK_VIEW_HPP
#define SPIRE_BLOTTER_TASK_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's tasks. */
  class BlotterTaskView : public QWidget {
    public:

      /**
       * Constructs a BlotterTaskView.
       * @param parent The parent widget.
       */
      explicit BlotterTaskView(QWidget* parent = nullptr);

    private:
      Box* m_box;
  };
}

#endif
