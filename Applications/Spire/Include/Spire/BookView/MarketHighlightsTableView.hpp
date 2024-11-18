#ifndef SPIRE_MARKET_HIGHLIGHTS_TABLE_VIEW_HPP
#define SPIRE_MARKET_HIGHLIGHTS_TABLE_VIEW_HPP
#include "Nexus/Definitions/Market.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** The type of model used for a list of MarketHighlight. */
  using MarketHighlightListModel =
    ListModel<BookViewHighlightProperties::MarketHighlight>;

  /**
   * Returns a new EditableTableView for the market highlights.
   * @param market_highlights The list of market highlights.
   * @param markets The market database to use.
   * @param parent The parent widget.
   */
  TableView* make_market_highlights_table_view(
    std::shared_ptr<MarketHighlightListModel> market_highlights,
    Nexus::MarketDatabase markets, QWidget* parent = nullptr);
}

#endif
