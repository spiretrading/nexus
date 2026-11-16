#ifndef SPIRE_BLOTTER_POSITIONS_VIEW_HPP
#define SPIRE_BLOTTER_POSITIONS_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a blotter's positions. */
  class BlotterPositionsView : public QWidget {
    public:

      /**
       * Constructs an empty BlotterPositionsView.
       * @param parent The parent widget.
       */
      explicit BlotterPositionsView(QWidget* parent = nullptr);
  };
}

#endif
