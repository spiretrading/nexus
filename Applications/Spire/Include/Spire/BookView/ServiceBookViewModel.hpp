#ifndef SPIRE_SERVICE_BOOK_VIEW_MODEL_HPP
#define SPIRE_SERVICE_BOOK_VIEW_MODEL_HPP
#include <unordered_map>
#include <vector>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/ReversedListModel.hpp"

namespace Spire {

  /**
   * Implements a BookViewModel using ServiceClient calls.
   */
  class ServiceBookViewModel : public BookViewModel {
    public:

      /**
       * Constructs a ServiceBookViewModel for a given security.
       * @param security The Security whose order book is to be modeled.
       * @param blotter The blotter used to keep track of tasks on the given
       *        <i>security</i>.
       * @param client The client used to access market data.
       */
      ServiceBookViewModel(Nexus::Security security, BlotterSettings& blotter,
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
      const std::shared_ptr<SecurityTechnicalsModel>&
        get_technicals() const override;

    private:
      Nexus::Security m_security;
      BlotterSettings* m_blotter;
      Nexus::MarketDataClient m_client;
      std::shared_ptr<BookViewModel> m_model;
      std::shared_ptr<BookQuoteListModel> m_bid_quotes;
      std::shared_ptr<BookQuoteListModel> m_ask_quotes;
      std::vector<Nexus::BookQuote> m_buffered_book_quotes;
      std::unordered_map<Nexus::Venue, Nexus::Venue> m_venue_quotes;
      std::vector<std::shared_ptr<Nexus::Order>> m_ask_orders;
      std::vector<std::shared_ptr<Nexus::Order>> m_bid_orders;
      std::shared_ptr<QtPromise<void>> m_load_promise;
      EventHandler m_event_handler;
      boost::optional<EventHandler> m_order_event_handler;
      boost::signals2::scoped_connection m_order_added_connection;
      boost::signals2::scoped_connection m_order_removed_connection;
      boost::signals2::scoped_connection m_active_blotter_connection;

      void clear(const BookQuoteListModel& quotes);
      void on_bbo(const Nexus::BboQuote& quote);
      void buffer_book_quote(const Nexus::BookQuote& quote);
      void on_end_book_quote_buffer();
      void on_book_quote(const Nexus::BookQuote& quote);
      void on_book_quote_interruption(const std::exception_ptr& e);
      void on_time_and_sales(const Nexus::TimeAndSale& time_and_sale);
      void on_execution_report(const std::shared_ptr<Nexus::Order>& order,
        const Nexus::ExecutionReport& report);
      void on_order_added(const OrderLogModel::OrderEntry& order);
      void on_order_removed(const OrderLogModel::OrderEntry& order);
      void on_active_blotter(BlotterModel& blotter);
  };
}

#endif
