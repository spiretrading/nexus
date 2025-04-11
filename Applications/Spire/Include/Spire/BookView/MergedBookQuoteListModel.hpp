#ifndef SPIRE_MERGED_BOOK_QUOTE_LIST_MODEL_HPP
#define SPIRE_MERGED_BOOK_QUOTE_LIST_MODEL_HPP
#include <boost/circular_buffer.hpp>
#include <boost/variant/variant.hpp>
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"

namespace Spire {

  /**
   * Stores one of the types of values that can be displayed in a BookView,
   * namely a book quote representing market data, an order submitted by a user,
   * or an order being previewed for submission.
   */
  using BookListing = boost::variant<Nexus::BookQuote,
    BookViewModel::UserOrder, Nexus::OrderExecutionService::OrderFields>;

  /**
   * Merges a list of book quotes, user submitted orders and the preview order
   * into one single list suitable for display in the BookViewWindow.
   */
  class MergedBookQuoteListModel : public ListModel<BookListing> {
    public:

      /**
       * Constructs a MergedBookQuoteListModel where the BookQuotes are listed
       * first followed by the list of UserOrders and finally the preview.
       * @param book_quotes The list of BookQuotes at the head of this list.
       * @param user_orders The list of user_orders to append to the
       *        <i>book_quotes</i>.
       * @param preview The preview order to display at the end of this list.
       */
      MergedBookQuoteListModel(std::shared_ptr<BookQuoteListModel> book_quotes,
        std::shared_ptr<BookViewModel::UserOrderListModel> user_orders,
        std::shared_ptr<BookViewModel::PreviewOrderModel> preview);

      int get_size() const override;

      const Type& get(int index) const;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<BookQuoteListModel> m_book_quotes;
      std::shared_ptr<BookViewModel::UserOrderListModel> m_user_orders;
      std::shared_ptr<BookViewModel::PreviewOrderModel> m_preview;
      BookViewModel::PreviewOrderModel::Type m_previous_preview;
      mutable boost::circular_buffer<BookListing> m_reads;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_book_quotes_connection;
      boost::signals2::scoped_connection m_user_orders_connection;
      boost::signals2::scoped_connection m_preview_connection;

      void on_book_quote_operation(
        const BookQuoteListModel::Operation& operation);
      void on_user_order_operation(
        const BookViewModel::UserOrderListModel::Operation& operation);
      void on_preview(const
        boost::optional<Nexus::OrderExecutionService::OrderFields>& preview);
  };
}

#endif
