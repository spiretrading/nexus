#ifndef NEXUS_MARKET_DATA_REGISTRY_SERVLET_HPP
#define NEXUS_MARKET_DATA_REGISTRY_SERVLET_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/MarketDataService/MarketDataRegistrySession.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus::MarketDataService {

  /**
   * Maintains a registry of all Securities and data subscriptions.
   * @param <C> The container instantiating this servlet.
   * @param <R> The registry storing all market data originating from this
   *        servlet.
   * @param <D> The type of data store storing historical market data.
   * @param <A> The type of AdministrationClient to use.
   */
  template<typename C, typename R, typename D, typename A>
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
       * @param administrationClient Used to check for entitlements.
       * @param marketDataRegistry The registry storing all market data
       *        originating from this servlet.
       * @param dataStore Initializes the historical market data store.
       */
      template<typename AF, typename RF, typename DF>
      MarketDataRegistryServlet(AF&& administrationClient,
        RF&& marketDataRegistry, DF&& dataStore);

      void Add(const SecurityInfo& securityInfo);

      void PublishOrderImbalance(const MarketOrderImbalance& orderImbalance,
        int sourceId);

      void PublishBboQuote(const SecurityBboQuote& bboQuote, int sourceId);

      void PublishMarketQuote(const SecurityMarketQuote& marketQuote,
        int sourceId);

      void UpdateBookQuote(const SecurityBookQuote& delta, int sourceId);

      void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale,
        int sourceId);

      void Clear(int sourceId);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      template<typename T>
      using MarketSubscriptions = Beam::Queries::IndexedSubscriptions<
        T, MarketCode, ServiceProtocolClient>;
      template<typename T>
      using SecuritySubscriptions =
        Beam::Queries::IndexedSubscriptions<T, Security, ServiceProtocolClient>;
      EntitlementDatabase m_entitlementDatabase;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<R> m_registry;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      MarketSubscriptions<OrderImbalance> m_orderImbalanceSubscriptions;
      SecuritySubscriptions<BboQuote> m_bboQuoteSubscriptions;
      SecuritySubscriptions<BookQuote> m_bookQuoteSubscriptions;
      SecuritySubscriptions<MarketQuote> m_marketQuoteSubscriptions;
      SecuritySubscriptions<TimeAndSale> m_timeAndSaleSubscriptions;
      Beam::IO::OpenState m_openState;

      MarketDataRegistryServlet(const MarketDataRegistryServlet&) = delete;
      MarketDataRegistryServlet& operator =(
        const MarketDataRegistryServlet&) = delete;
      Security NormalizePrimaryMarket(const Security& security);
      void OnQueryOrderImbalances(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryOrderImbalancesService>& request,
        const MarketWideDataQuery& query);
      void OnEndOrderImbalanceQuery(ServiceProtocolClient& client,
        MarketCode market, int id);
      void OnQueryBboQuotes(Beam::Services::RequestToken<ServiceProtocolClient,
        QueryBboQuotesService>& request, const SecurityMarketDataQuery& query);
      void OnEndBboQuoteQuery(ServiceProtocolClient& client,
        const Security& security, int id);
      void OnQueryBookQuotes(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryBookQuotesService>& request,
        const SecurityMarketDataQuery& query);
      void OnEndBookQuoteQuery(ServiceProtocolClient& client,
        const Security& security, int id);
      void OnQueryMarketQuotes(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryMarketQuotesService>& request,
        const SecurityMarketDataQuery& query);
      void OnEndMarketQuoteQuery(ServiceProtocolClient& client,
        const Security& security, int id);
      void OnQueryTimeAndSales(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryTimeAndSalesService>& request,
        const SecurityMarketDataQuery& query);
      void OnEndTimeAndSaleQuery(ServiceProtocolClient& client,
        const Security& security, int id);
      SecuritySnapshot OnLoadSecuritySnapshot(ServiceProtocolClient& client,
        Security security);
      SecurityTechnicals OnLoadSecurityTechnicals(ServiceProtocolClient& client,
        Security security);
      std::vector<SecurityInfo> OnQuerySecurityInfo(
        ServiceProtocolClient& client, const SecurityInfoQuery& query);
      std::vector<SecurityInfo> OnLoadSecurityInfoFromPrefix(
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

  template<typename C, typename R, typename D, typename A>
  template<typename AF, typename RF, typename DF>
  MarketDataRegistryServlet<C, R, D, A>::MarketDataRegistryServlet(
      AF&& administrationClient, RF&& registry, DF&& dataStore)
      : m_administrationClient(std::forward<AF>(administrationClient)),
        m_registry(std::forward<RF>(registry)),
        m_dataStore(std::forward<DF>(dataStore)) {
    try {
      auto query = SecurityInfoQuery();
      query.SetIndex(Region::Global());
      query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
      auto securityInfo = m_dataStore->LoadSecurityInfo(query);
      for(auto& entry : securityInfo) {
        m_registry->Add(entry);
      }
      m_entitlementDatabase = m_administrationClient->LoadEntitlements();
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::Add(
      const SecurityInfo& securityInfo) {
    m_dataStore->Store(securityInfo);
    m_registry->Add(securityInfo);
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::PublishOrderImbalance(
      const MarketOrderImbalance& orderImbalance, int sourceId) {
    m_registry->PublishOrderImbalance(orderImbalance, sourceId, *m_dataStore,
      [&] (const auto& orderImbalance) {
        m_dataStore->Store(orderImbalance);
        m_orderImbalanceSubscriptions.Publish(orderImbalance,
          [&] (const auto& clients) {
            Beam::Services::BroadcastRecordMessage<OrderImbalanceMessage>(
              clients, orderImbalance);
          });
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::PublishBboQuote(
      const SecurityBboQuote& bboQuote, int sourceId) {
    m_registry->PublishBboQuote(bboQuote, sourceId, *m_dataStore,
      [&] (const auto& bboQuote) {
        m_dataStore->Store(bboQuote);
        m_bboQuoteSubscriptions.Publish(bboQuote, [&] (const auto& clients) {
          Beam::Services::BroadcastRecordMessage<BboQuoteMessage>(clients,
            bboQuote);
        });
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::PublishMarketQuote(
      const SecurityMarketQuote& marketQuote, int sourceId) {
    m_registry->PublishMarketQuote(marketQuote, sourceId, *m_dataStore,
      [&] (const auto& marketQuote) {
        m_dataStore->Store(marketQuote);
        m_marketQuoteSubscriptions.Publish(marketQuote,
          [&] (const auto& clients) {
            Beam::Services::BroadcastRecordMessage<MarketQuoteMessage>(clients,
              marketQuote);
          });
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::UpdateBookQuote(
      const SecurityBookQuote& delta, int sourceId) {
    auto security = m_registry->GetPrimaryListing(delta.GetIndex());
    auto key = EntitlementKey(security.GetMarket(), delta.GetValue().m_market);
    m_registry->UpdateBookQuote(delta, sourceId, *m_dataStore,
      [&] (const auto& bookQuote) {
        m_dataStore->Store(bookQuote);
        if(security.GetMarket() == MarketCode()) {
          return;
        }
        m_bookQuoteSubscriptions.Publish(bookQuote,
          [&] (const auto& client) {
            return HasEntitlement(client.GetSession(), key,
              MarketDataType::BOOK_QUOTE);
          },
          [&] (const auto& clients) {
            Beam::Services::BroadcastRecordMessage<BookQuoteMessage>(clients,
              bookQuote);
          });
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::PublishTimeAndSale(
      const SecurityTimeAndSale& timeAndSale, int sourceId) {
    m_registry->PublishTimeAndSale(timeAndSale, sourceId, *m_dataStore,
      [&] (const auto& timeAndSale) {
        m_dataStore->Store(timeAndSale);
        m_timeAndSaleSubscriptions.Publish(timeAndSale,
          [&] (const auto& clients) {
            Beam::Services::BroadcastRecordMessage<TimeAndSaleMessage>(clients,
              timeAndSale);
          });
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::Clear(int sourceId) {
    m_registry->Clear(sourceId);
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterMarketDataRegistryServices(Beam::Store(slots));
    RegisterMarketDataRegistryMessages(Beam::Store(slots));
    QueryOrderImbalancesService::AddRequestSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::OnQueryOrderImbalances, this));
    Beam::Services::AddMessageSlot<EndOrderImbalanceQueryMessage>(
      Store(slots), std::bind_front(
        &MarketDataRegistryServlet::OnEndOrderImbalanceQuery, this));
    QueryBboQuotesService::AddRequestSlot(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnQueryBboQuotes, this));
    Beam::Services::AddMessageSlot<EndBboQuoteQueryMessage>(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnEndBboQuoteQuery, this));
    QueryBookQuotesService::AddRequestSlot(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnQueryBookQuotes, this));
    Beam::Services::AddMessageSlot<EndBookQuoteQueryMessage>(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnEndBookQuoteQuery, this));
    QueryMarketQuotesService::AddRequestSlot(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnQueryMarketQuotes, this));
    Beam::Services::AddMessageSlot<EndMarketQuoteQueryMessage>(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnEndMarketQuoteQuery, this));
    QueryTimeAndSalesService::AddRequestSlot(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnQueryTimeAndSales, this));
    Beam::Services::AddMessageSlot<EndTimeAndSaleQueryMessage>(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnEndTimeAndSaleQuery, this));
    LoadSecuritySnapshotService::AddSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::OnLoadSecuritySnapshot, this));
    LoadSecurityTechnicalsService::AddSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::OnLoadSecurityTechnicals, this));
    QuerySecurityInfoService::AddSlot(Store(slots),
      std::bind_front(&MarketDataRegistryServlet::OnQuerySecurityInfo, this));
    LoadSecurityInfoFromPrefixService::AddSlot(Store(slots), std::bind_front(
      &MarketDataRegistryServlet::OnLoadSecurityInfoFromPrefix, this));
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    session.m_roles = m_administrationClient->LoadAccountRoles(
      session.GetAccount());
    auto& entitlements = m_entitlementDatabase.GetEntries();
    auto accountEntitlements = m_administrationClient->LoadEntitlements(
      session.GetAccount());
    for(auto& entitlement : entitlements) {
      auto entryIterator = std::find(accountEntitlements.begin(),
        accountEntitlements.end(), entitlement.m_groupEntry);
      if(entryIterator != accountEntitlements.end()) {
        for(auto& applicability : entitlement.m_applicability) {
          session.m_entitlements.GrantEntitlement(applicability.first,
            applicability.second);
        }
      }
    }
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_orderImbalanceSubscriptions.RemoveAll(client);
    m_bboQuoteSubscriptions.RemoveAll(client);
    m_marketQuoteSubscriptions.RemoveAll(client);
    m_bookQuoteSubscriptions.RemoveAll(client);
    m_timeAndSaleSubscriptions.RemoveAll(client);
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_openState.Close();
  }

  template<typename C, typename R, typename D, typename A>
  Security MarketDataRegistryServlet<C, R, D, A>::NormalizePrimaryMarket(
      const Security& security) {
    if(security.GetMarket().IsEmpty()) {
      return security;
    }
    auto result =
      m_dataStore->LoadSecurityInfo(MakeSecurityInfoQuery(security));
    if(result.empty()) {
      return security;
    }
    if(result.front().m_security.GetMarket() == security.GetMarket()) {
      return result.front().m_security;
    }
    return {};
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnQueryOrderImbalances(
      Beam::Services::RequestToken<ServiceProtocolClient,
        QueryOrderImbalancesService>& request,
      const MarketWideDataQuery& query) {
    auto& session = request.GetSession();
    if(!HasEntitlement(session, query.GetIndex(),
        MarketDataType::ORDER_IMBALANCE)) {
      request.SetResult(OrderImbalanceQueryResult());
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    auto result = OrderImbalanceQueryResult();
    result.m_queryId = m_orderImbalanceSubscriptions.Initialize(
      query.GetIndex(), request.GetClient(), query.GetRange(),
      std::move(filter));
    result.m_snapshot = m_dataStore->LoadOrderImbalances(query);
    m_orderImbalanceSubscriptions.Commit(query.GetIndex(), std::move(result),
      [&] (const auto& result) {
        request.SetResult(result);
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnEndOrderImbalanceQuery(
      ServiceProtocolClient& client, MarketCode market, int id) {
    m_orderImbalanceSubscriptions.End(market, id);
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnQueryBboQuotes(
      Beam::Services::RequestToken<ServiceProtocolClient,
        QueryBboQuotesService>& request, const SecurityMarketDataQuery& query) {
    auto& session = request.GetSession();
    if(!HasEntitlement(session, query.GetIndex().GetMarket(),
        MarketDataType::BBO_QUOTE)) {
      request.SetResult(BboQuoteQueryResult());
      return;
    }
    auto security = NormalizePrimaryMarket(query.GetIndex());
    if(security == Security()) {
      request.SetResult(BboQuoteQueryResult());
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    auto result = BboQuoteQueryResult();
    result.m_queryId = m_bboQuoteSubscriptions.Initialize(security,
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore->LoadBboQuotes(query);
    m_bboQuoteSubscriptions.Commit(security, std::move(result),
      [&] (const auto& result) {
        request.SetResult(result);
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnEndBboQuoteQuery(
      ServiceProtocolClient& client, const Security& security, int id) {
    m_bboQuoteSubscriptions.End(security, id);
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnQueryBookQuotes(
      Beam::Services::RequestToken<ServiceProtocolClient,
        QueryBookQuotesService>& request,
      const SecurityMarketDataQuery& query) {
    auto& session = request.GetSession();
    if(!HasEntitlement(session, query.GetIndex().GetMarket(),
        MarketDataType::BOOK_QUOTE)) {
      request.SetResult(BookQuoteQueryResult());
      return;
    }
    auto security = NormalizePrimaryMarket(query.GetIndex());
    if(security == Security()) {
      request.SetResult(BookQuoteQueryResult());
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    auto result = BookQuoteQueryResult();
    result.m_queryId = m_bookQuoteSubscriptions.Initialize(security,
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore->LoadBookQuotes(query);
    m_bookQuoteSubscriptions.Commit(security, std::move(result),
      [&] (const auto& result) {
        request.SetResult(result);
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnEndBookQuoteQuery(
      ServiceProtocolClient& client, const Security& security, int id) {
    m_bookQuoteSubscriptions.End(security, id);
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnQueryMarketQuotes(
      Beam::Services::RequestToken<ServiceProtocolClient,
        QueryMarketQuotesService>& request,
      const SecurityMarketDataQuery& query) {
    auto& session = request.GetSession();
    if(!HasEntitlement(session, query.GetIndex().GetMarket(),
        MarketDataType::MARKET_QUOTE)) {
      request.SetResult(MarketQuoteQueryResult());
      return;
    }
    auto security = NormalizePrimaryMarket(query.GetIndex());
    if(security == Security()) {
      request.SetResult(MarketQuoteQueryResult());
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    auto result = MarketQuoteQueryResult();
    result.m_queryId = m_marketQuoteSubscriptions.Initialize(security,
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore->LoadMarketQuotes(query);
    m_marketQuoteSubscriptions.Commit(security, std::move(result),
      [&] (const auto& result) {
        request.SetResult(result);
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnEndMarketQuoteQuery(
      ServiceProtocolClient& client, const Security& security, int id) {
    m_marketQuoteSubscriptions.End(security, id);
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnQueryTimeAndSales(
      Beam::Services::RequestToken<ServiceProtocolClient,
        QueryTimeAndSalesService>& request,
      const SecurityMarketDataQuery& query) {
    auto& session = request.GetSession();
    if(!HasEntitlement(session, query.GetIndex().GetMarket(),
        MarketDataType::TIME_AND_SALE)) {
      request.SetResult(TimeAndSaleQueryResult());
      return;
    }
    auto security = NormalizePrimaryMarket(query.GetIndex());
    if(security == Security()) {
      request.SetResult(TimeAndSaleQueryResult());
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    auto result = TimeAndSaleQueryResult();
    result.m_queryId = m_timeAndSaleSubscriptions.Initialize(security,
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore->LoadTimeAndSales(query);
    m_timeAndSaleSubscriptions.Commit(security, std::move(result),
      [&] (const auto& result) {
        request.SetResult(result);
      });
  }

  template<typename C, typename R, typename D, typename A>
  void MarketDataRegistryServlet<C, R, D, A>::OnEndTimeAndSaleQuery(
      ServiceProtocolClient& client, const Security& security, int id) {
    m_timeAndSaleSubscriptions.End(security, id);
  }

  template<typename C, typename R, typename D, typename A>
  SecuritySnapshot MarketDataRegistryServlet<C, R, D, A>::
      OnLoadSecuritySnapshot(ServiceProtocolClient& client, Security security) {
    auto& session = client.GetSession();
    security = NormalizePrimaryMarket(security);
    if(security == Security()) {
      return {};
    }
    auto securitySnapshot = m_registry->FindSnapshot(security);
    if(!securitySnapshot) {
      return {};
    }
    if(!HasEntitlement(session, security.GetMarket(),
        MarketDataType::BBO_QUOTE)) {
      securitySnapshot->m_bboQuote = SequencedBboQuote();
    }
    if(!HasEntitlement(session, security.GetMarket(),
        MarketDataType::TIME_AND_SALE)) {
      securitySnapshot->m_timeAndSale = SequencedTimeAndSale();
    }
    if(!HasEntitlement(session, security.GetMarket(),
        MarketDataType::MARKET_QUOTE)) {
      securitySnapshot->m_marketQuotes.clear();
    }
    auto askEndRange = std::remove_if(securitySnapshot->m_askBook.begin(),
      securitySnapshot->m_askBook.end(),
      [&] (auto& bookQuote) {
        return !HasEntitlement(session,
          EntitlementKey(security.GetMarket(), bookQuote->m_market),
          MarketDataType::BOOK_QUOTE);
      });
    securitySnapshot->m_askBook.erase(askEndRange,
      securitySnapshot->m_askBook.end());
    auto bidEndRange = std::remove_if(securitySnapshot->m_bidBook.begin(),
      securitySnapshot->m_bidBook.end(), [&] (auto& bookQuote) {
        return !HasEntitlement(session,
          EntitlementKey(security.GetMarket(), bookQuote->m_market),
          MarketDataType::BOOK_QUOTE);
      });
    securitySnapshot->m_bidBook.erase(bidEndRange,
      securitySnapshot->m_bidBook.end());
    return *securitySnapshot;
  }

  template<typename C, typename R, typename D, typename A>
  SecurityTechnicals MarketDataRegistryServlet<C, R, D, A>::
      OnLoadSecurityTechnicals(ServiceProtocolClient& client,
        Security security) {
    security = NormalizePrimaryMarket(security);
    if(auto securityTechnicals = m_registry->FindSecurityTechnicals(security)) {
      return *securityTechnicals;
    }
    return {};
  }

  template<typename C, typename R, typename D, typename A>
  std::vector<SecurityInfo> MarketDataRegistryServlet<C, R, D, A>::
      OnQuerySecurityInfo(ServiceProtocolClient& client,
        const SecurityInfoQuery& query) {
    return m_dataStore->LoadSecurityInfo(query);
  }

  template<typename C, typename R, typename D, typename A>
  std::vector<SecurityInfo> MarketDataRegistryServlet<C, R, D, A>::
      OnLoadSecurityInfoFromPrefix(ServiceProtocolClient& client,
        const std::string& prefix) {
    return m_registry->SearchSecurityInfo(prefix);
  }
}

#endif
