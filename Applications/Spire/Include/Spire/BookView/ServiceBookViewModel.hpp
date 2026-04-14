#ifndef SPIRE_SERVICE_BOOK_VIEW_MODEL_HPP
#define SPIRE_SERVICE_BOOK_VIEW_MODEL_HPP
#include <vector>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/BookView/LocalBookViewModel.hpp"

namespace Spire {

  /**
   * Implements a BookViewModel using ServiceClient calls.
   */
  class ServiceBookViewModel : public BookViewModel {
    public:

      /**
       * Constructs a ServiceBookViewModel for a given ticker.
       * @param ticker The Ticker whose order book is to be modeled.
       * @param blotter The blotter used to keep track of tasks on the given
       *        <i>ticker</i>.
       * @param client The client used to access market data.
       */
      ServiceBookViewModel(Nexus::Ticker ticker, BlotterSettings& blotter,
        Nexus::MarketDataClient client);

      const std::shared_ptr<BookQuoteListModel>& get_bids() const override;
      const std::shared_ptr<BookQuoteListModel>& get_asks() const override;
      const std::shared_ptr<UserOrderListModel>&
        get_bid_orders() const override;
      const std::shared_ptr<UserOrderListModel>&
        get_ask_orders() const override;
      const std::shared_ptr<PreviewOrderModel>&
        get_preview_order() const override;
      const std::shared_ptr<BboQuoteModel>& get_bbo_quote() const override;
      const std::shared_ptr<SessionCandlestickModel>&
        get_session_candlestick() const override;

    private:
      Nexus::Ticker m_ticker;
      BlotterSettings* m_blotter;
      Nexus::MarketDataClient m_client;
      LocalBookViewModel m_model;
      std::vector<Nexus::BookQuote> m_buffered_book_quotes;
      std::shared_ptr<QtPromise<void>> m_load_promise;
      EventHandler m_event_handler;
      boost::optional<EventHandler> m_order_event_handler;
      boost::signals2::scoped_connection m_order_added_connection;
      boost::signals2::scoped_connection m_order_removed_connection;
      boost::signals2::scoped_connection m_active_blotter_connection;

      void on_bbo(const Nexus::BboQuote& bbo);
      void buffer_book_quote(const Nexus::BookQuote& quote);
      void on_end_book_quote_buffer();
      void on_book_quote_interruption(const std::exception_ptr& e);
      void on_time_and_sales(const Nexus::TimeAndSale& time_and_sale);
      void on_execution_report(const Nexus::ExecutionReport& report);
      void on_order_added(const OrderLogModel::OrderEntry& order);
      void on_order_removed(const OrderLogModel::OrderEntry& order);
      void on_active_blotter(BlotterModel& blotter);
  };
}

#endif
