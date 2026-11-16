#ifndef SPIRE_BLOTTER_STATUS_BAR_HPP
#define SPIRE_BLOTTER_STATUS_BAR_HPP
#include <QWidget>

namespace Spire {

  /** Displays a blotter's summary values and message status. */
  class BlotterStatusBar : public QWidget {
    public:

      /**
       * Constructs an empty BlotterStatusBar.
       * @param parent The parent widget.
       */
      explicit BlotterStatusBar(QWidget* parent = nullptr);
  };
}

#endif
