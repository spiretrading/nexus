#ifndef SPIRE_TICKER_FILTER_PANEL_HPP
#define SPIRE_TICKER_FILTER_PANEL_HPP
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/TickerListBox.hpp"

namespace Spire {

  /** An OpenFilterPanel specialized for a TickerListBox. */
  using TickerFilterPanel = OpenFilterPanel<TickerListBox>;

  /**
   * Returns a new TickerFilterPanel with a default current model.
   * @param tickers The set of tickers that can be queried.
   * @param parent The parent widget.
   */
  TickerFilterPanel* make_ticker_filter_panel(
    std::shared_ptr<TickerInfoQueryModel> tickers, QWidget* parent = nullptr);

  /**
   * Returns a new TickerFilterPanel.
   * @param tickers The set of tickers that can be queried.
   * @param current The current list of selected tickers.
   * @param parent The parent widget.
   */
  TickerFilterPanel* make_ticker_filter_panel(
    std::shared_ptr<TickerInfoQueryModel> tickers,
    std::shared_ptr<TickerListModel> current, QWidget* parent = nullptr);
}

#endif
