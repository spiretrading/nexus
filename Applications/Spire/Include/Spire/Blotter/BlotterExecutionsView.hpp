#ifndef SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#define SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a blotter's executions. */
  class BlotterExecutionsView : public QWidget {
    public:

      /**
       * Constructs an empty BlotterExecutionsView.
       * @param parent The parent widget.
       */
      explicit BlotterExecutionsView(QWidget* parent = nullptr);
  };
}

#endif
