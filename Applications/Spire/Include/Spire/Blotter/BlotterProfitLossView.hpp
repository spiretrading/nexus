#ifndef SPIRE_BLOTTER_PROFIT_LOSS_VIEW_HPP
#define SPIRE_BLOTTER_PROFIT_LOSS_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/PortfolioModel.hpp"

namespace Spire {
  class TableView;

  /** Displays a blotter's profit and loss. */
  class BlotterProfitLossView : public QWidget {
    public:

      /**
       * Constructs a BlotterProfitLossView.
       * @param portfolio The portfolio whose profit and loss is displayed.
       * @param parent The parent widget.
       */
      explicit BlotterProfitLossView(
        std::shared_ptr<PortfolioModel> portfolio, QWidget* parent = nullptr);

      /** Returns the portfolio displayed. */
      const std::shared_ptr<PortfolioModel>& get_portfolio() const;

    private:
      friend class BlotterWindowSettings;
      std::shared_ptr<PortfolioModel> m_portfolio;
      TableView* m_table;

      void on_context_menu(const QPoint& position);
      void on_header_menu(const QPoint& position);
  };
}

#endif
