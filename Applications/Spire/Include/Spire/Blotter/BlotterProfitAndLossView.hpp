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
       * @param profit_and_loss The profit and losses per security to display.
       * @param parent The parent widget.
       */
      explicit BlotterProfitAndLossView(
        std::shared_ptr<BlotterProfitAndLossModel> profit_and_loss,
        QWidget* parent = nullptr);

      /** Returns the displayed profit and losses. */
      const std::shared_ptr<BlotterProfitAndLossModel>& get_profit_and_loss()
        const;

    private:
      std::shared_ptr<BlotterProfitAndLossModel> m_profit_and_loss;
  };
}

#endif
