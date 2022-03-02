#ifndef SPIRE_BLOTTER_STATUS_BAR_HPP
#define SPIRE_BLOTTER_STATUS_BAR_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's status bar. */
  class BlotterStatusBar : public QWidget {
    public:

      /**
       * Constructs a BlotterStatusBar.
       * @param parent The parent widget.
       */
      explicit BlotterStatusBar(QWidget* parent = nullptr);

    private:
      Box* m_box;
  };
}

#endif
