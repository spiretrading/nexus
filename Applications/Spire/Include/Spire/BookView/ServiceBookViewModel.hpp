#ifndef SPIRE_SERVICE_BOOK_VIEW_MODEL_HPP
#define SPIRE_SERVICE_BOOK_VIEW_MODEL_HPP
#include <unordered_map>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /**
   * Implements a BookViewModel using ServiceClient calls.
   */
  class ServiceBookViewModel : public BookViewModel {
    public:

      /**
       * Constructs a ServiceBookViewModel for a given security.
       * @param security The Security whose order book is to be modeled.
       * @param markets The database of market definitions.
       * @param client The client used to access market data.
       */
      ServiceBookViewModel(
        Nexus::Security security, Nexus::MarketDatabase markets,
        Nexus::MarketDataService::MarketDataClientBox client);

      const std::shared_ptr<BookQuoteListModel>& get_bids() const override;

      const std::shared_ptr<BookQuoteListModel>& get_asks() const override;

      const std::shared_ptr<UserOrderListModel>&
        get_bid_orders() const override;

      const std::shared_ptr<UserOrderListModel>&
        get_ask_orders() const override;

      const std::shared_ptr<PreviewOrderModel>&
        get_preview_order() const override;

      const std::shared_ptr<BboQuoteModel>& get_bbo_quote() const override;

      const std::shared_ptr<SecurityTechnicalsValueModel>&
        get_technicals() const override;

    private:
      Nexus::Security m_security;
      Nexus::MarketDatabase m_markets;
      Nexus::MarketDataService::MarketDataClientBox m_client;
      std::shared_ptr<BookViewModel> m_model;
      std::unordered_map<Nexus::MarketCode, Nexus::MarketQuote> m_market_quotes;
      EventHandler m_event_handler;

      void on_bbo(const Nexus::BboQuote& quote);
      void on_book_quote(const Nexus::BookQuote& quote);
      void on_book_quote_interruption(const std::exception_ptr& e);
      void on_market_quote(const Nexus::MarketQuote& quote);
      void on_market_quote_interruption(const std::exception_ptr& e);
  };
}

#endif
