#ifndef SPIRE_MERGED_BOOK_ENTRY_LIST_MODEL_HPP
#define SPIRE_MERGED_BOOK_ENTRY_LIST_MODEL_HPP
#include <deque>
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/TransactionLogModel.hpp"

namespace Spire {

  /**
   * Merges a list of book quotes, user submitted orders and the preview order
   * into one single list of BookEntries.
   */
  class MergedBookEntryListModel :
      public TransactionLogModel<BookEntryListModel> {
    public:

      /**
       * Constructs a MergedBookEntryListModel where the BookQuotes are listed
       * first followed by the list of UserOrders and finally the preview.
       * @param book_quotes The list of BookQuotes at the head of this list.
       * @param user_orders The list of user_orders to append to the
       *        <i>book_quotes</i>.
       * @param preview The preview order to display at the end of this list.
       */
      MergedBookEntryListModel(std::shared_ptr<BookQuoteListModel> book_quotes,
        std::shared_ptr<BookViewModel::UserOrderListModel> user_orders,
        std::shared_ptr<BookViewModel::PreviewOrderModel> preview);

      int get_size() const override;
      const Type& get(int index) const override;

    private:
      std::shared_ptr<BookQuoteListModel> m_book_quotes;
      std::shared_ptr<BookViewModel::UserOrderListModel> m_user_orders;
      std::shared_ptr<BookViewModel::PreviewOrderModel> m_preview;
      BookViewModel::PreviewOrderModel::Type m_previous_preview;
      std::deque<BookEntry> m_entries;
      boost::signals2::scoped_connection m_book_quotes_connection;
      boost::signals2::scoped_connection m_user_orders_connection;
      boost::signals2::scoped_connection m_preview_connection;

      template<typename T>
      void apply(const ListModel<T>::Operation& operation,
        const ListModel<T>& source, int offset);
      void on_book_quote_operation(
        const BookQuoteListModel::Operation& operation);
      void on_user_order_operation(
        const BookViewModel::UserOrderListModel::Operation& operation);
      void on_preview(const boost::optional<Nexus::OrderFields>& preview);
  };

  /**
   * Moves a BookEntry to a new position within a list of BookEntries.
   * @param entries The list of BookEntries to update.
   * @param source The index of the BookEntry to move.
   * @param destination The index to move the BookEntry to.
   */
  void move_book_entry(
    std::deque<BookEntry>& entries, int source, int destination);
}

#endif
