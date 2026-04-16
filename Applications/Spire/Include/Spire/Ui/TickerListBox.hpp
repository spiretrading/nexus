#ifndef SPIRE_TICKER_LIST_BOX_HPP
#define SPIRE_TICKER_LIST_BOX_HPP
#include "Spire/Spire/TickerInfoToTickerQueryModel.hpp"
#include "Spire/Ui/TagComboBox.hpp"

namespace Spire {

  /** Represents a ListModel for the Ticker. */
  using TickerListModel = ListModel<Nexus::Ticker>;

  /** A TagComboBox specialized for a Nexus::Ticker. */
  using TickerListBox = TagComboBox<Nexus::Ticker>;

  /**
   * Returns a new TickerListBox with a default current model.
   * @param tickers The set of tickers that can be queried.
   * @param parent The parent widget.
   */
  TickerListBox* make_ticker_list_box(
    std::shared_ptr<TickerInfoQueryModel> tickers,
    QWidget* parent = nullptr);

  /**
   * Returns a new TickerListBox.
   * @param tickers The set of tickers that can be queried.
   * @param current The current list of selected tickers.
   * @param parent The parent widget.
   */
  TickerListBox* make_ticker_list_box(
    std::shared_ptr<TickerInfoQueryModel> tickers,
    std::shared_ptr<TickerListModel> current,
    QWidget* parent = nullptr);
}

#endif
