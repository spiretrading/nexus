#ifndef SPIRE_BLOTTER_PROFIT_AND_LOSS_VIEW_HPP
#define SPIRE_BLOTTER_PROFIT_AND_LOSS_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's profit and loss. */
  class BlotterProfitAndLossView : public QWidget {
    public:

      /**
       * Constructs a BlotterProfitAndLossView.
       * @param parent The parent widget.
       */
      explicit BlotterProfitAndLossView(QWidget* parent = nullptr);

    private:
      Box* m_box;
  };
}

#endif
