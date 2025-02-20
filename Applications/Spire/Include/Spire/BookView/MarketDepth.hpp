#ifndef SPIRE_MARKET_DEPTH_HPP
#define SPIRE_MARKET_DEPTH_HPP
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ValueModel over optional BookQuote. */
  using BookQuoteModel = ValueModel<boost::optional<Nexus::BookQuote>>;

  /**
   * Displays a widget that contains the BBO panel along with tables for
   * bids and asks.
   */
  class MarketDepth : public QWidget {
    public:

      /**
       * Constructs a MarketDepth.
       * @param model The book view model.
       * @param bbo_quote The best bid and ask Quotes across all markets.
       * @param properties The properties applied to the table view.
       * @param markets The market database.
       * @param parent The parent widget.
       */
      MarketDepth(std::shared_ptr<BookViewModel> model,
        std::shared_ptr<ValueModel<Nexus::BboQuote>> bbo_quote,
        std::shared_ptr<BookViewPropertiesModel> properties,
        const Nexus::MarketDatabase& markets, QWidget* parent = nullptr);

      /** Returns the currently selected book quote. */
      const std::shared_ptr<BookQuoteModel>& get_selected_book_quote() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;

    private:
      std::shared_ptr<BookViewModel> m_model;
      std::shared_ptr<BookQuoteModel> m_selected_quote;
      std::shared_ptr<ValueModel<QFont>> m_font_property;
      QFont m_font;
      TableView* m_bid_table_view;
      TableView* m_ask_table_view;
      boost::optional<int> m_last_bid_current_row;
      boost::optional<int> m_last_ask_current_row;
      boost::optional<int> m_last_current_row;
      boost::signals2::scoped_connection m_bid_current_connection;
      boost::signals2::scoped_connection m_ask_current_connection;
      boost::signals2::scoped_connection m_bid_operation_connection;
      boost::signals2::scoped_connection m_ask_operation_connection;
      boost::signals2::scoped_connection m_bid_position_connection;
      boost::signals2::scoped_connection m_ask_position_connection;
      boost::signals2::scoped_connection m_font_property_connection;

      void on_side_current(const boost::optional<TableView::Index>& current,
        Nexus::Side side);
      void on_side_operation(const TableModel::Operation& operation,
        Nexus::Side side);
      void on_bid_position(int position);
      void on_ask_position(int position);
      void on_bid_current(const boost::optional<TableView::Index>& current);
      void on_ask_current(const boost::optional<TableView::Index>& current);
      void on_bid_operation(const TableModel::Operation& operation);
      void on_ask_operation(const TableModel::Operation& operation);
      void on_font_property_update(const QFont& font);
  };
}

#endif
