#ifndef SPIRE_VENUE_HIGHLIGHTS_TABLE_VIEW_HPP
#define SPIRE_VENUE_HIGHLIGHTS_TABLE_VIEW_HPP
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** The type of model used for a list of VenueHighlight. */
  using VenueHighlightListModel =
    ListModel<BookViewHighlightProperties::VenueHighlight>;

  /**
   * Returns a new EditableTableView for the venue highlights.
   * @param venue_highlights The list of venue highlights.
   * @param parent The parent widget.
   */
  TableView* make_venue_highlights_table_view(
    std::shared_ptr<VenueHighlightListModel> venue_highlights,
    QWidget* parent = nullptr);
}

#endif
