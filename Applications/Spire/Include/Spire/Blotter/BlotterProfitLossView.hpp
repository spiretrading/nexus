#ifndef SPIRE_BLOTTER_PROFIT_LOSS_VIEW_HPP
#define SPIRE_BLOTTER_PROFIT_LOSS_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a blotter's profit and loss. */
  class BlotterProfitLossView : public QWidget {
    public:

      /**
       * Constructs an empty BlotterProfitLossView.
       * @param parent The parent widget.
       */
      explicit BlotterProfitLossView(QWidget* parent = nullptr);
  };
}

#endif
