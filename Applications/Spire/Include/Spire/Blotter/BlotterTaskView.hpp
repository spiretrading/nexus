#ifndef SPIRE_BLOTTER_TASK_VIEW_HPP
#define SPIRE_BLOTTER_TASK_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays task details for a blotter. */
  class BlotterTaskView : public QWidget {
    public:

      /**
       * Constructs an empty BlotterTaskView.
       * @param parent The parent widget.
       */
      explicit BlotterTaskView(QWidget* parent = nullptr);
  };
}

#endif
