#ifndef NEXUS_MARKET_DATA_REGISTRY_SERVLET_HPP
#define NEXUS_MARKET_DATA_REGISTRY_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/MarketDataService/MarketDataRegistrySession.hpp"
#include "Nexus/MarketDataService/TickerMarketDataQuery.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {

  /**
   * Maintains a registry of all Tickers and data subscriptions.
   * @param <C> The container instantiating this servlet.
   * @param <R> The registry storing all market data originating from this
   *        servlet.
   * @param <D> The type of data store storing historical market data.
   * @param <A> The type of AdministrationClient to use.
   */
  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  class MarketDataRegistryServlet {
    public:

      /** The registry storing all market data sent to this servlet. */
      using MarketDataRegistry = Beam::dereference_t<R>;

      /** The type of data store storing historical market data. */
      using HistoricalDataStore = Beam::dereference_t<D>;

      /** The type of AdministrationClient to use. */
      using AdministrationClient = Beam::dereference_t<A>;

      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a MarketDataRegistryServlet.
       * @param administration_client Used to check for entitlements.
       * @param market_data_registry The registry storing all market data
       *        originating from this servlet.
       * @param data_store Initializes the historical market data store.
       */
      template<Beam::Initializes<A> AF, Beam::Initializes<R> RF,
        Beam::Initializes<D> DF>
      MarketDataRegistryServlet(AF&& administration_client,
        RF&& market_data_registry, DF&& data_store);

      void add(const TickerInfo& info);
      void publish(const VenueOrderImbalance& imbalance, int source_id);
      void publish(const TickerBboQuote& quote, int source_id);
      void publish(const TickerBookQuote& delta, int source_id);
      void publish(const TickerTimeAndSale& time_and_sale, int source_id);
      void clear(int source_id);
      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void handle_accept(ServiceProtocolClient& client);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      template<typename T>
      using VenueSubscriptions =
        Beam::IndexedSubscriptions<T, Venue, ServiceProtocolClient>;
      template<typename T>
      using TickerSubscriptions =
        Beam::IndexedSubscriptions<T, Ticker, ServiceProtocolClient>;
      EntitlementDatabase m_entitlement_database;
      Beam::local_ptr_t<A> m_administration_client;
      Beam::local_ptr_t<R> m_registry;
      Beam::local_ptr_t<D> m_data_store;
      VenueSubscriptions<OrderImbalance> m_order_imbalance_subscriptions;
      TickerSubscriptions<BboQuote> m_bbo_quote_subscriptions;
      TickerSubscriptions<BookQuote> m_book_quote_subscriptions;
      TickerSubscriptions<TimeAndSale> m_time_and_sale_subscriptions;
      Beam::OpenState m_open_state;

      MarketDataRegistryServlet(const MarketDataRegistryServlet&) = delete;
      MarketDataRegistryServlet& operator =(
        const MarketDataRegistryServlet&) = delete;
      Ticker normalize(const Ticker& ticker);
      Venue normalize(Venue venue);
      template<typename Type, typename Service, typename Query,
        typename Subscriptions>
      void on_query(Beam::RequestToken<ServiceProtocolClient, Service>& request,
        const Query& query, Subscriptions& subscriptions);
      void on_query_order_imbalance(Beam::RequestToken<
        ServiceProtocolClient, QueryOrderImbalancesService>& request,
        const VenueMarketDataQuery& query);
      void on_end_order_imbalance_query(
        ServiceProtocolClient& client, Venue venue, int id);
      void on_query_bbo_quotes(Beam::RequestToken<
        ServiceProtocolClient, QueryBboQuotesService>& request,
        const TickerMarketDataQuery& query);
      void on_end_bbo_quote_query(
        ServiceProtocolClient& client, const Ticker& ticker, int id);
      void on_query_book_quotes(Beam::RequestToken<
        ServiceProtocolClient, QueryBookQuotesService>& request,
        const TickerMarketDataQuery& query);
      void on_end_book_quote_query(
        ServiceProtocolClient& client, const Ticker& ticker, int id);
      void on_query_time_and_sales(Beam::RequestToken<
        ServiceProtocolClient, QueryTimeAndSalesService>& request,
        const TickerMarketDataQuery& query);
      void on_end_time_and_sale_query(
        ServiceProtocolClient& client, const Ticker& ticker, int id);
      TickerSnapshot on_load_ticker_snapshot(
        ServiceProtocolClient& client, Ticker ticker);
      PriceCandlestick on_load_session_candlestick(
        ServiceProtocolClient& client, Ticker ticker);
      std::vector<TickerInfo> on_query_ticker_info(
        ServiceProtocolClient& client, const TickerInfoQuery& query);
      std::vector<TickerInfo> on_load_ticker_info_from_prefix(
        ServiceProtocolClient& client, const std::string& prefix);
  };

  template<typename R, typename D, typename A>
  struct MetaMarketDataRegistryServlet {
    using Session = MarketDataRegistrySession;

    template<typename C>
    struct apply {
      using type = MarketDataRegistryServlet<C, R, D, A>;
    };
  };

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<Beam::Initializes<A> AF, Beam::Initializes<R> RF,
    Beam::Initializes<D> DF>
  MarketDataRegistryServlet<C, R, D, A>::MarketDataRegistryServlet(
      AF&& administration_client, RF&& registry, DF&& data_store)
      : m_administration_client(std::forward<AF>(administration_client)),
        m_registry(std::forward<RF>(registry)),
        m_data_store(std::forward<DF>(data_store)) {
    try {
      auto query = TickerInfoQuery();
      query.set_index(Scope::GLOBAL);
      query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
      auto info = m_data_store->load_ticker_info(query);
      for(auto& entry : info) {
        m_registry->add(entry);
      }
      m_entitlement_database = m_administration_client->load_entitlements();
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::add(const TickerInfo& info) {
    m_data_store->store(info);
    m_registry->add(info);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::publish(
      const VenueOrderImbalance& imbalance, int source_id) {
    m_registry->publish(imbalance, source_id, *m_data_store,
      [&] (const auto& imbalance) {
        m_data_store->store(imbalance);
        m_order_imbalance_subscriptions.publish(imbalance,
          [&] (const auto& clients) {
            Beam::broadcast_record_message<OrderImbalanceMessage>(
              clients, imbalance);
          });
      });
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::publish(
      const TickerBboQuote& quote, int source_id) {
    m_registry->publish(quote, source_id, *m_data_store,
      [&] (const auto& quote) {
        m_data_store->store(quote);
        m_bbo_quote_subscriptions.publish(quote,
          [&] (const auto& clients) {
            Beam::broadcast_record_message<BboQuoteMessage>(clients, quote);
          });
      });
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::publish(
      const TickerBookQuote& delta, int source_id) {
    auto ticker = m_registry->get_primary_listing(delta.get_index());
    auto key = EntitlementKey(ticker.get_venue(), delta.get_value().m_venue);
    m_registry->publish(delta, source_id, *m_data_store,
      [&] (const auto& quote) {
        m_data_store->store(quote);
        if(!ticker.get_venue()) {
          return;
        }
        m_book_quote_subscriptions.publish(quote, [&] (const auto& client) {
          return has_entitlement(
            client.get_session(), key, MarketDataType::BOOK_QUOTE);
        },
        [&] (const auto& clients) {
          Beam::broadcast_record_message<BookQuoteMessage>(clients, quote);
        });
      });
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::publish(
      const TickerTimeAndSale& time_and_sale, int source_id) {
    m_registry->publish(time_and_sale, source_id, *m_data_store,
      [&] (const auto& time_and_sale) {
        m_data_store->store(time_and_sale);
        m_time_and_sale_subscriptions.publish(time_and_sale,
          [&] (const auto& clients) {
            Beam::broadcast_record_message<TimeAndSaleMessage>(
              clients, time_and_sale);
          });
      });
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::clear(int source_id) {
    m_registry->clear(source_id);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::register_services(
      Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    Nexus::register_query_types(Beam::out(slots->get_registry()));
    register_market_data_registry_services(out(slots));
    register_market_data_registry_messages(out(slots));
    QueryOrderImbalancesService::add_request_slot(out(slots), std::bind_front(
      &MarketDataRegistryServlet::on_query_order_imbalance, this));
    Beam::add_message_slot<EndOrderImbalanceQueryMessage>(
      out(slots), std::bind_front(
        &MarketDataRegistryServlet::on_end_order_imbalance_query, this));
    QueryBboQuotesService::add_request_slot(out(slots),
      std::bind_front(&MarketDataRegistryServlet::on_query_bbo_quotes, this));
    Beam::add_message_slot<EndBboQuoteQueryMessage>(
      out(slots), std::bind_front(
        &MarketDataRegistryServlet::on_end_bbo_quote_query, this));
    QueryBookQuotesService::add_request_slot(out(slots),
      std::bind_front(&MarketDataRegistryServlet::on_query_book_quotes, this));
    Beam::add_message_slot<EndBookQuoteQueryMessage>(
      out(slots), std::bind_front(
        &MarketDataRegistryServlet::on_end_book_quote_query, this));
    QueryTimeAndSalesService::add_request_slot(out(slots),
      std::bind_front(&MarketDataRegistryServlet::on_query_time_and_sales,
        this));
    Beam::add_message_slot<EndTimeAndSaleQueryMessage>(
      out(slots), std::bind_front(
        &MarketDataRegistryServlet::on_end_time_and_sale_query, this));
    LoadTickerSnapshotService::add_slot(out(slots), std::bind_front(
      &MarketDataRegistryServlet::on_load_ticker_snapshot, this));
    LoadSessionCandlestickService::add_slot(out(slots), std::bind_front(
      &MarketDataRegistryServlet::on_load_session_candlestick, this));
    QueryTickerInfoService::add_slot(out(slots), std::bind_front(
      &MarketDataRegistryServlet::on_query_ticker_info, this));
    LoadTickerInfoFromPrefixService::add_slot(out(slots), std::bind_front(
      &MarketDataRegistryServlet::on_load_ticker_info_from_prefix, this));
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::handle_accept(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    session.m_roles =
      m_administration_client->load_account_roles(session.get_account());
    auto account_entitlements =
      m_administration_client->load_entitlements(session.get_account());
    for(auto& entitlement : m_entitlement_database.get_entries()) {
      auto i = std::find(account_entitlements.begin(),
        account_entitlements.end(), entitlement.m_group_entry);
      if(i != account_entitlements.end()) {
        for(auto& applicability : entitlement.m_applicability) {
          session.m_entitlements.grant(
            applicability.first, applicability.second);
        }
      }
    }
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::handle_close(
      ServiceProtocolClient& client) {
    m_order_imbalance_subscriptions.remove_all(client);
    m_bbo_quote_subscriptions.remove_all(client);
    m_book_quote_subscriptions.remove_all(client);
    m_time_and_sale_subscriptions.remove_all(client);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_data_store->close();
    m_open_state.close();
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  Ticker MarketDataRegistryServlet<C, R, D, A>::normalize(
      const Ticker& ticker) {
    if(!ticker.get_venue()) {
      return ticker;
    }
    auto result =
      m_data_store->load_ticker_info(make_ticker_info_query(ticker));
    if(result.empty()) {
      return ticker;
    }
    if(result.front().m_ticker.get_venue() == ticker.get_venue()) {
      return result.front().m_ticker;
    }
    return {};
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  Venue MarketDataRegistryServlet<C, R, D, A>::normalize(Venue venue) {
    return venue;
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  template<typename Type, typename Service, typename Query,
    typename Subscriptions>
  void MarketDataRegistryServlet<C, R, D, A>::on_query(
      Beam::RequestToken<ServiceProtocolClient, Service>& request,
      const Query& query, Subscriptions& subscriptions) {
    using Result = Beam::QueryResult<Beam::SequencedValue<Type>>;
    auto& session = request.get_session();
    if(!has_entitlement<Type>(session, query)) {
      request.set(Result());
      return;
    }
    auto index = normalize(query.get_index());
    if(index == typename Query::Index()) {
      request.set(Result());
      return;
    }
    auto filter = Beam::translate<EvaluatorTranslator>(query.get_filter());
    auto result = Result();
    result.m_id = subscriptions.init(
      index, request.get_client(), query.get_range(), std::move(filter));
    result.m_snapshot = load<Type>(*m_data_store, query);
    subscriptions.commit(index, std::move(result), [&] (const auto& result) {
      request.set(result);
    });
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::on_query_order_imbalance(
      Beam::RequestToken<ServiceProtocolClient, QueryOrderImbalancesService>&
        request, const VenueMarketDataQuery& query) {
    on_query<OrderImbalance>(request, query, m_order_imbalance_subscriptions);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::on_end_order_imbalance_query(
      ServiceProtocolClient& client, Venue venue, int id) {
    m_order_imbalance_subscriptions.end(venue, id);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::on_query_bbo_quotes(
      Beam::RequestToken<ServiceProtocolClient, QueryBboQuotesService>& request,
      const TickerMarketDataQuery& query) {
    on_query<BboQuote>(request, query, m_bbo_quote_subscriptions);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::on_end_bbo_quote_query(
      ServiceProtocolClient& client, const Ticker& ticker, int id) {
    m_bbo_quote_subscriptions.end(ticker, id);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::on_query_book_quotes(
      Beam::RequestToken<ServiceProtocolClient, QueryBookQuotesService>&
        request, const TickerMarketDataQuery& query) {
    on_query<BookQuote>(request, query, m_book_quote_subscriptions);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::on_end_book_quote_query(
      ServiceProtocolClient& client, const Ticker& ticker, int id) {
    m_book_quote_subscriptions.end(ticker, id);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::on_query_time_and_sales(
      Beam::RequestToken<ServiceProtocolClient, QueryTimeAndSalesService>&
        request, const TickerMarketDataQuery& query) {
    on_query<TimeAndSale>(request, query, m_time_and_sale_subscriptions);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  void MarketDataRegistryServlet<C, R, D, A>::on_end_time_and_sale_query(
      ServiceProtocolClient& client, const Ticker& ticker, int id) {
    m_time_and_sale_subscriptions.end(ticker, id);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  TickerSnapshot MarketDataRegistryServlet<C, R, D, A>::on_load_ticker_snapshot(
      ServiceProtocolClient& client, Ticker ticker) {
    auto& session = client.get_session();
    ticker = normalize(ticker);
    if(!ticker) {
      return {};
    }
    auto snapshot = m_registry->find_snapshot(ticker);
    if(!snapshot) {
      return {};
    }
    if(!has_entitlement(session,
        EntitlementKey(ticker.get_venue()), MarketDataType::BBO_QUOTE)) {
      snapshot->m_bbo_quote = SequencedBboQuote();
    }
    if(!has_entitlement(session,
        EntitlementKey(ticker.get_venue()), MarketDataType::TIME_AND_SALE)) {
      snapshot->m_time_and_sale = SequencedTimeAndSale();
    }
    auto ask_end_range = std::remove_if(snapshot->m_asks.begin(),
      snapshot->m_asks.end(), [&] (const auto& quote) {
        return !has_entitlement(
          session, EntitlementKey(ticker.get_venue(), quote->m_venue),
          MarketDataType::BOOK_QUOTE);
      });
    snapshot->m_asks.erase(ask_end_range, snapshot->m_asks.end());
    auto bid_end_range = std::remove_if(snapshot->m_bids.begin(),
      snapshot->m_bids.end(), [&] (const auto& quote) {
        return !has_entitlement(
          session, EntitlementKey(ticker.get_venue(), quote->m_venue),
          MarketDataType::BOOK_QUOTE);
      });
    snapshot->m_bids.erase(bid_end_range, snapshot->m_bids.end());
    return *snapshot;
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  PriceCandlestick MarketDataRegistryServlet<C, R, D, A>::
      on_load_session_candlestick(
        ServiceProtocolClient& client, Ticker ticker) {
    ticker = normalize(ticker);
    if(auto candlestick = m_registry->find_session_candlestick(ticker)) {
      return *candlestick;
    }
    return {};
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  std::vector<TickerInfo> MarketDataRegistryServlet<C, R, D, A>::
      on_query_ticker_info(
        ServiceProtocolClient& client, const TickerInfoQuery& query) {
    return m_data_store->load_ticker_info(query);
  }

  template<typename C, typename R, typename D, typename A> requires
    IsHistoricalDataStore<Beam::dereference_t<D>> &&
      IsAdministrationClient<Beam::dereference_t<A>>
  std::vector<TickerInfo> MarketDataRegistryServlet<C, R, D, A>::
      on_load_ticker_info_from_prefix(
        ServiceProtocolClient& client, const std::string& prefix) {
    return m_registry->search_ticker_info(prefix);
  }
}

#endif
