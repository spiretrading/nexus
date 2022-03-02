#ifndef SPIRE_BLOTTER_ORDER_LOG_VIEW_HPP
#define SPIRE_BLOTTER_ORDER_LOG_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's order log. */
  class BlotterOrderLogView : public QWidget {
    public:

      /**
       * Constructs a BlotterOrderLogView.
       * @param parent The parent widget.
       */
      explicit BlotterOrderLogView(QWidget* parent = nullptr);

    private:
      Box* m_box;
  };
}

#endif
