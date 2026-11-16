#ifndef SPIRE_BLOTTER_ORDER_LOG_VIEW_HPP
#define SPIRE_BLOTTER_ORDER_LOG_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a blotter's order log. */
  class BlotterOrderLogView : public QWidget {
    public:

      /**
       * Constructs an empty BlotterOrderLogView.
       * @param parent The parent widget.
       */
      explicit BlotterOrderLogView(QWidget* parent = nullptr);
  };
}

#endif
