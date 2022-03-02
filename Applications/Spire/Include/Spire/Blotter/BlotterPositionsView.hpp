#ifndef SPIRE_BLOTTER_POSITIONS_VIEW_HPP
#define SPIRE_BLOTTER_POSITIONS_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's positions. */
  class BlotterPositionsView : public QWidget {
    public:

      /**
       * Constructs a BlotterPositionsView.
       * @param parent The parent widget.
       */
      explicit BlotterPositionsView(QWidget* parent = nullptr);

    private:
      Box* m_box;
  };
}

#endif
