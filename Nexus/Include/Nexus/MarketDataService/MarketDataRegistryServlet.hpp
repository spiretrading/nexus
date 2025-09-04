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
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {

  /**
   * Maintains a registry of all Securities and data subscriptions.
   * @param <C> The container instantiating this servlet.
   * @param <R> The registry storing all market data originating from this
   *        servlet.
   * @param <D> The type of data store storing historical market data.
   * @param <A> The type of AdministrationClient to use.
   */
  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  class MarketDataRegistryServlet {
    public:

      /** The registry storing all market data sent to this servlet. */
      using MarketDataRegistry = Beam::GetTryDereferenceType<R>;

      /** The type of data store storing historical market data. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of AdministrationClient to use. */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

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

      void add(const SecurityInfo& info);
      void publish(const VenueOrderImbalance& imbalance, int source_id);
      void publish(const SecurityBboQuote& quote, int source_id);
      void publish(const SecurityBookQuote& delta, int source_id);
      void publish(const SecurityTimeAndSale& time_and_sale, int source_id);
      void clear(int source_id);

      void RegisterServices(
        Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      template<typename T>
      using VenueSubscriptions =
        Beam::Queries::IndexedSubscriptions<T, Venue, ServiceProtocolClient>;
      template<typename T>
      using SecuritySubscriptions =
        Beam::Queries::IndexedSubscriptions<T, Security, ServiceProtocolClient>;
      EntitlementDatabase m_entitlement_database;
      Beam::GetOptionalLocalPtr<A> m_administration_client;
      Beam::GetOptionalLocalPtr<R> m_registry;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      VenueSubscriptions<OrderImbalance> m_order_imbalance_subscriptions;
      SecuritySubscriptions<BboQuote> m_bbo_quote_subscriptions;
      SecuritySubscriptions<BookQuote> m_book_quote_subscriptions;
      SecuritySubscriptions<TimeAndSale> m_time_and_sale_subscriptions;
      Beam::IO::OpenState m_open_state;

      MarketDataRegistryServlet(const MarketDataRegistryServlet&) = delete;
      MarketDataRegistryServlet& operator =(
        const MarketDataRegistryServlet&) = delete;
      Security normalize(const Security& security);
      Venue normalize(Venue venue);
      template<typename Type, typename Service, typename Query,
        typename Subscriptions>
      void on_query(
          Beam::Services::RequestToken<ServiceProtocolClient, Service>& request,
          const Query& query, Subscriptions& subscriptions);
      void on_query_order_imbalance(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryOrderImbalancesService>& request,
        const VenueMarketDataQuery& query);
      void on_end_order_imbalance_query(
        ServiceProtocolClient& client, Venue venue, int id);
      void on_query_bbo_quotes(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryBboQuotesService>& request,
        const SecurityMarketDataQuery& query);
      void on_end_bbo_quote_query(
        ServiceProtocolClient& client, const Security& security, int id);
      void on_query_book_quotes(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryBookQuotesService>& request,
        const SecurityMarketDataQuery& query);
      void on_end_book_quote_query(
        ServiceProtocolClient& client, const Security& security, int id);
      void on_query_time_and_sales(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryTimeAndSalesService>& request,
        const SecurityMarketDataQuery& query);
      void on_end_time_and_sale_query(
        ServiceProtocolClient& client, const Security& security, int id);
      SecuritySnapshot on_load_security_snapshot(
        ServiceProtocolClient& client, Security security);
      SecurityTechnicals on_load_security_technicals(
        ServiceProtocolClient& client, Security security);
      std::vector<SecurityInfo> on_query_security_info(
        ServiceProtocolClient& client, const SecurityInfoQuery& query);
      std::vector<SecurityInfo> on_load_security_info_from_prefix(
        ServiceProtocolClient& client, const std::string& prefix);
  };

  template<typename R, IsHistoricalDataStore D, IsAdministrationClient A>
  struct MetaMarketDataRegistryServlet {
    using Session = MarketDataRegistrySession;
    template<typename C>
    struct apply {
      using type = MarketDataRegistryServlet<C, R, D, A>;
    };
  };

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  template<Beam::Initializes<A> AF, Beam::Initializes<R> RF,
    Beam::Initializes<D> DF>
  MarketDataRegistryServlet<C, R, D, A>::MarketDataRegistryServlet(
      AF&& administration_client, RF&& registry, DF&& data_store)
      : m_administration_client(std::forward<AF>(administration_client)),
        m_registry(std::forward<RF>(registry)),
        m_data_store(std::forward<DF>(data_store)) {
    try {
      auto query = SecurityInfoQuery();
      query.SetIndex(Region::GLOBAL);
      query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
      auto info = m_data_store->load_security_info(query);
      for(auto& entry : info) {
        m_registry->add(entry);
      }
      m_entitlement_database = m_administration_client->load_entitlements();
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::add(const SecurityInfo& info) {
    m_data_store->store(info);
    m_registry->add(info);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::publish(
      const VenueOrderImbalance& imbalance, int source_id) {
    m_registry->publish(imbalance, source_id, *m_data_store,
      [&] (const auto& imbalance) {
        m_data_store->store(imbalance);
        m_order_imbalance_subscriptions.Publish(imbalance,
          [&] (const auto& clients) {
            Beam::Services::BroadcastRecordMessage<OrderImbalanceMessage>(
              clients, imbalance);
          });
      });
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::publish(
      const SecurityBboQuote& quote, int source_id) {
    m_registry->publish(quote, source_id, *m_data_store,
      [&] (const auto& quote) {
        m_data_store->store(quote);
        m_bbo_quote_subscriptions.Publish(quote,
          [&] (const auto& clients) {
            Beam::Services::BroadcastRecordMessage<BboQuoteMessage>(
              clients, quote);
          });
      });
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::publish(
      const SecurityBookQuote& delta, int source_id) {
    auto security = m_registry->get_primary_listing(delta.GetIndex());
    auto key = EntitlementKey(security.get_venue(), delta.GetValue().m_venue);
    m_registry->publish(delta, source_id, *m_data_store,
      [&] (const auto& quote) {
        m_data_store->store(quote);
        if(!security.get_venue()) {
          return;
        }
        m_book_quote_subscriptions.Publish(quote, [&] (const auto& client) {
          return has_entitlement(
            client.GetSession(), key, MarketDataType::BOOK_QUOTE);
        },
        [&] (const auto& clients) {
          Beam::Services::BroadcastRecordMessage<BookQuoteMessage>(
            clients, quote);
        });
      });
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::publish(
      const SecurityTimeAndSale& time_and_sale, int source_id) {
    m_registry->publish(time_and_sale, source_id, *m_data_store,
      [&] (const auto& time_and_sale) {
        m_data_store->store(time_and_sale);
        m_time_and_sale_subscriptions.Publish(time_and_sale,
          [&] (const auto& clients) {
            Beam::Services::BroadcastRecordMessage<TimeAndSaleMessage>(
              clients, time_and_sale);
          });
      });
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::clear(int source_id) {
    m_registry->clear(source_id);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterMarketDataRegistryServices(Store(slots));
    RegisterMarketDataRegistryMessages(Store(slots));
    QueryOrderImbalancesService::AddRequestSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::on_query_order_imbalance, this));
    Beam::Services::AddMessageSlot<EndOrderImbalanceQueryMessage>(
      Store(slots), std::bind_front(
        &MarketDataRegistryServlet::on_end_order_imbalance_query, this));
    QueryBboQuotesService::AddRequestSlot(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::on_query_bbo_quotes, this));
    Beam::Services::AddMessageSlot<EndBboQuoteQueryMessage>(
      Store(slots), std::bind_front(
        &MarketDataRegistryServlet::on_end_bbo_quote_query, this));
    QueryBookQuotesService::AddRequestSlot(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::on_query_book_quotes, this));
    Beam::Services::AddMessageSlot<EndBookQuoteQueryMessage>(
      Store(slots), std::bind_front(
        &MarketDataRegistryServlet::on_end_book_quote_query, this));
    QueryTimeAndSalesService::AddRequestSlot(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::on_query_time_and_sales,
        this));
    Beam::Services::AddMessageSlot<EndTimeAndSaleQueryMessage>(
      Store(slots), std::bind_front(
        &MarketDataRegistryServlet::on_end_time_and_sale_query, this));
    LoadSecuritySnapshotService::AddSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::on_load_security_snapshot, this));
    LoadSecurityTechnicalsService::AddSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::on_load_security_technicals, this));
    QuerySecurityInfoService::AddSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::on_query_security_info, this));
    LoadSecurityInfoFromPrefixService::AddSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::on_load_security_info_from_prefix, this));
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    session.m_roles =
      m_administration_client->load_account_roles(session.GetAccount());
    auto account_entitlements =
      m_administration_client->load_entitlements(session.GetAccount());
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

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_order_imbalance_subscriptions.RemoveAll(client);
    m_bbo_quote_subscriptions.RemoveAll(client);
    m_book_quote_subscriptions.RemoveAll(client);
    m_time_and_sale_subscriptions.RemoveAll(client);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::Close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_data_store->close();
    m_open_state.Close();
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  Security MarketDataRegistryServlet<C, R, D, A>::normalize(
      const Security& security) {
    if(!security.get_venue()) {
      return security;
    }
    auto result =
      m_data_store->load_security_info(make_security_info_query(security));
    if(result.empty()) {
      return security;
    }
    if(result.front().m_security.get_venue() == security.get_venue()) {
      return result.front().m_security;
    }
    return {};
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  Venue MarketDataRegistryServlet<C, R, D, A>::normalize(Venue venue) {
    return venue;
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  template<typename Type, typename Service, typename Query,
    typename Subscriptions>
  void MarketDataRegistryServlet<C, R, D, A>::on_query(
      Beam::Services::RequestToken<ServiceProtocolClient, Service>& request,
      const Query& query, Subscriptions& subscriptions) {
    using Result =
      Beam::Queries::QueryResult<Beam::Queries::SequencedValue<Type>>;
    auto& session = request.GetSession();
    if(!has_entitlement<Type>(session, query)) {
      request.SetResult(Result());
      return;
    }
    auto index = normalize(query.GetIndex());
    if(index == typename Query::Index()) {
      request.SetResult(Result());
      return;
    }
    auto filter =
      Beam::Queries::Translate<EvaluatorTranslator>(query.GetFilter());
    auto result = Result();
    result.m_queryId = subscriptions.Initialize(
      index, request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = load<Type>(*m_data_store, query);
    subscriptions.Commit(index, std::move(result), [&] (const auto& result) {
      request.SetResult(result);
    });
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::on_query_order_imbalance(
      Beam::Services::RequestToken<ServiceProtocolClient,
        QueryOrderImbalancesService>& request,
      const VenueMarketDataQuery& query) {
    on_query<OrderImbalance>(request, query, m_order_imbalance_subscriptions);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::on_end_order_imbalance_query(
      ServiceProtocolClient& client, Venue venue, int id) {
    m_order_imbalance_subscriptions.End(venue, id);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::on_query_bbo_quotes(
      Beam::Services::RequestToken<ServiceProtocolClient,
        QueryBboQuotesService>& request, const SecurityMarketDataQuery& query) {
    on_query<BboQuote>(request, query, m_bbo_quote_subscriptions);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::on_end_bbo_quote_query(
      ServiceProtocolClient& client, const Security& security, int id) {
    m_bbo_quote_subscriptions.End(security, id);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::on_query_book_quotes(
      Beam::Services::RequestToken<
        ServiceProtocolClient, QueryBookQuotesService>& request,
      const SecurityMarketDataQuery& query) {
    on_query<BookQuote>(request, query, m_book_quote_subscriptions);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::on_end_book_quote_query(
      ServiceProtocolClient& client, const Security& security, int id) {
    m_book_quote_subscriptions.End(security, id);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::on_query_time_and_sales(
      Beam::Services::RequestToken<
        ServiceProtocolClient, QueryTimeAndSalesService>& request,
      const SecurityMarketDataQuery& query) {
    on_query<TimeAndSale>(request, query, m_time_and_sale_subscriptions);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  void MarketDataRegistryServlet<C, R, D, A>::on_end_time_and_sale_query(
      ServiceProtocolClient& client, const Security& security, int id) {
    m_time_and_sale_subscriptions.End(security, id);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  SecuritySnapshot MarketDataRegistryServlet<C, R, D, A>::
      on_load_security_snapshot(
        ServiceProtocolClient& client, Security security) {
    auto& session = client.GetSession();
    security = normalize(security);
    if(!security) {
      return {};
    }
    auto snapshot = m_registry->find_snapshot(security);
    if(!snapshot) {
      return {};
    }
    if(!has_entitlement(session,
        EntitlementKey(security.get_venue()), MarketDataType::BBO_QUOTE)) {
      snapshot->m_bbo_quote = SequencedBboQuote();
    }
    if(!has_entitlement(session,
        EntitlementKey(security.get_venue()), MarketDataType::TIME_AND_SALE)) {
      snapshot->m_time_and_sale = SequencedTimeAndSale();
    }
    auto ask_end_range = std::remove_if(snapshot->m_asks.begin(),
      snapshot->m_asks.end(), [&] (const auto& quote) {
        return !has_entitlement(
          session, EntitlementKey(security.get_venue(), quote->m_venue),
          MarketDataType::BOOK_QUOTE);
      });
    snapshot->m_asks.erase(ask_end_range, snapshot->m_asks.end());
    auto bid_end_range = std::remove_if(snapshot->m_bids.begin(),
      snapshot->m_bids.end(), [&] (const auto& quote) {
        return !has_entitlement(
          session, EntitlementKey(security.get_venue(), quote->m_venue),
          MarketDataType::BOOK_QUOTE);
      });
    snapshot->m_bids.erase(bid_end_range, snapshot->m_bids.end());
    return *snapshot;
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  SecurityTechnicals MarketDataRegistryServlet<C, R, D, A>::
      on_load_security_technicals(
        ServiceProtocolClient& client, Security security) {
    security = normalize(security);
    if(auto technicals = m_registry->find_security_technicals(security)) {
      return *technicals;
    }
    return {};
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  std::vector<SecurityInfo> MarketDataRegistryServlet<C, R, D, A>::
      on_query_security_info(
        ServiceProtocolClient& client, const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  template<typename C, typename R, IsHistoricalDataStore D,
    IsAdministrationClient A>
  std::vector<SecurityInfo> MarketDataRegistryServlet<C, R, D, A>::
      on_load_security_info_from_prefix(
        ServiceProtocolClient& client, const std::string& prefix) {
    return m_registry->search_security_info(prefix);
  }
}

#endif
