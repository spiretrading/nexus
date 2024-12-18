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
   * @param parent The parent widget.
   */
  TableView* make_book_view_table_view(
    std::shared_ptr<BookViewModel> model,
    std::shared_ptr<BookViewPropertiesModel> properties,
    Nexus::Side side, const Nexus::MarketDatabase& markets, QWidget* parent = nullptr);

  /**
   * Implements a TableView for displaying the book view.
   */
  //class BookViewTableView : public TableView {
  //  public:

  //    /**
  //     * Constructs a BookViewTableView.
  //     * @param table The table model of book view.
  //     * @param properties The properties applied to the table view.
  //     * @param parent The parent widget.
  //     */
  //    BookViewTableView(std::shared_ptr<BookViewModel> model,
  //      std::shared_ptr<BookViewPropertiesModel> properties,
  //      Nexus::Side side, QWidget* parent = nullptr);

  //  protected:
  //    void resizeEvent(QResizeEvent* event) override;
  //    //void showEvent(QShowEvent* event) override;

  //  private:
  //    struct LevelQuoteModel;
  //    std::shared_ptr<BookViewModel> m_model;
  //    std::shared_ptr<BookViewPropertiesModel> m_properties;
  //    std::shared_ptr<ListModel<Nexus::BookQuote>> m_book_quotes;
  //    std::shared_ptr<LevelQuoteModel> m_level_quote_model;
  //    std::shared_ptr<LevelPropertiesModel> m_level_properties;
  //    boost::signals2::scoped_connection m_level_properties_connection;

  //    std::shared_ptr<ListModel<Nexus::BookQuote>> get_book_quotes(std::shared_ptr<BookViewModel> model, Nexus::Side side);
  //    std::shared_ptr<LevelQuoteModel> make_level_quote_model();
  //    QWidget* item_builder(const std::shared_ptr<TableModel>& table,
  //      int row, int column);
  //    void on_level_properties_update(const BookViewLevelProperties& properties);
  //};
}

#endif
