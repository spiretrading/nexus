#ifndef SPIRE_BOOK_VIEW_TABLE_VIEW_HPP
#define SPIRE_BOOK_VIEW_TABLE_VIEW_HPP
#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /**
   * Returns a new TableView for displaying the book view.
   * @param model The book view model.
   * @param properties The properties applied to the table view.
   * @param side The order side.
   * @param markets The market database.
   * @param parent The parent widget.
   */
  TableView* make_book_view_table_view(
    std::shared_ptr<BookViewModel> model,
    std::shared_ptr<BookViewPropertiesModel> properties,
    Nexus::Side side, const Nexus::MarketDatabase& markets,
    QWidget* parent = nullptr);
}

#endif
