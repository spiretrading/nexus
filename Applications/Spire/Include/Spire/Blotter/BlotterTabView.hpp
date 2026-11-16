#ifndef SPIRE_BLOTTER_TAB_VIEW_HPP
#define SPIRE_BLOTTER_TAB_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a blotter's positions, order log, executions, and profit/loss. */
  class BlotterTabView : public QWidget {
    public:

      /**
       * Constructs an empty BlotterTabView.
       * @param parent The parent widget.
       */
      explicit BlotterTabView(QWidget* parent = nullptr);
  };
}

#endif
