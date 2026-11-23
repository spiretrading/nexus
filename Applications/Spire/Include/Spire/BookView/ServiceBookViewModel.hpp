#ifndef SPIRE_SERVICE_BOOK_VIEW_MODEL_HPP
#define SPIRE_SERVICE_BOOK_VIEW_MODEL_HPP
#include <vector>
#include <QTimer>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Blotter/OrdersModel.hpp"
#include "Spire/BookView/LocalBookViewModel.hpp"

namespace Spire {

  /** Implements a BookViewModel using ServiceClient calls. */
  class ServiceBookViewModel : public BookViewModel {
    public:

      /**
       * Constructs a ServiceBookViewModel for a given ticker.
       * @param ticker The Ticker whose order book is to be modeled.
       * @param market_data_client The client used to access market data.
       * @param orders The orders to display.
       */
      ServiceBookViewModel(Nexus::Ticker ticker,
        Nexus::MarketDataClient market_data_client,
        std::shared_ptr<const OrdersModel> orders);

      const std::shared_ptr<BookQuoteListModel>& get_bids() const override;
      const std::shared_ptr<BookQuoteListModel>& get_asks() const override;
      const std::shared_ptr<UserOrderListModel>&
        get_bid_orders() const override;
      const std::shared_ptr<UserOrderListModel>&
        get_ask_orders() const override;
      const std::shared_ptr<PreviewOrderModel>&
        get_preview_order() const override;
      const std::shared_ptr<BboQuoteModel>& get_bbo_quote() const override;
      const std::shared_ptr<SessionTechnicalsModel>&
        get_session_technicals() const override;

    private:
      struct OrderEntry;
      Nexus::Ticker m_ticker;
      Nexus::MarketDataClient m_market_data_client;
      std::shared_ptr<const OrdersModel> m_orders;
      LocalBookViewModel m_model;
      std::vector<std::shared_ptr<OrderEntry>> m_order_entries;
      std::vector<Nexus::BookQuote> m_buffered_book_quotes;
      QTimer m_book_quote_retry_timer;
      QTimer m_time_and_sale_retry_timer;
      EventHandler m_event_handler;
      QtTaskQueue m_order_events;
      boost::signals2::scoped_connection m_orders_connection;

      void add(int index, std::shared_ptr<Nexus::Order> order);
      void remove(int index);
      void query_book_quotes();
      void query_time_and_sales();
      void buffer_book_quote(const Nexus::BookQuote& quote);
      void on_order_operation(const OrdersModel::Operation& operation);
      void on_bbo(const Nexus::BboQuote& bbo);
      void on_end_book_quote_buffer();
      void on_book_quote_interruption(const std::exception_ptr& e);
      void on_session_technicals(const Nexus::SessionTechnicals& technicals);
      void on_time_and_sales(const Nexus::TimeAndSale& time_and_sale);
      void on_time_and_sales_interruption(const std::exception_ptr& e);
  };
}

#endif
