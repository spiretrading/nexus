#ifndef NEXUS_MARKET_DATA_RELAY_SERVLET_HPP
#define NEXUS_MARKET_DATA_RELAY_SERVLET_HPP
#include <functional>
#include <vector>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include <Beam/Utilities/ResourcePool.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataClientUtilities.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/MarketDataService/MarketDataRegistrySession.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a relay servlet for querying market data.
   * @param C container instantiating this servlet.
   * @param M The type of MarketDataClient connected to the source providing
   *          market data queries.
   * @param A The type of AdministrationClient to use.
   */
  template<typename C, typename M, typename A>
  class MarketDataRelayServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * The type of MarketDataClient connected to the source providing market
       * data queries.
       */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /** The type of AdministrationClient to use. */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of function used to builds MarketDataClients. */
      using MarketDataClientBuilder =
        std::function<std::unique_ptr<MarketDataClient> ()>;

      /**
       * Constructs a MarketDataRelayServlet.
       * @param clientTimeout The amount of time to wait before building another
       *        MarketDataClient.
       * @param marketDataClientBuilder Constructs MarketDataClients used to
       *        distribute queries.
       * @param minMarketDataClients The minimum number of MarketDataClients to
       *        pool.
       * @param maxMarketDataClients The maximum number of MarketDataClients to
       *        pool.
       * @param administrationClient Used to check for entitlements.
       */
      template<typename AF>
      MarketDataRelayServlet(boost::posix_time::time_duration clientTimeout,
        MarketDataClientBuilder marketDataClientBuilder,
        std::size_t minMarketDataClients, std::size_t maxMarketDataClients,
        AF&& administrationClient);

      void RegisterServices(
        Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      struct RealTimeQueryEntry {
        std::unique_ptr<MarketDataClient> m_marketDataClient;
        Beam::RoutineTaskQueue m_tasks;

        RealTimeQueryEntry(std::unique_ptr<MarketDataClient> marketDataClient);
      };
      template<typename T>
      using MarketSubscriptions = Beam::Queries::IndexedSubscriptions<
        T, MarketCode, ServiceProtocolClient>;
      template<typename T>
      using SecuritySubscriptions =
        Beam::Queries::IndexedSubscriptions<T, Security, ServiceProtocolClient>;
      using RealTimeMarketSubscriptionSet =
        Beam::SynchronizedUnorderedSet<MarketCode, Beam::Threading::Mutex>;
      using RealTimeSecuritySubscriptionSet = Beam::SynchronizedSet<
        PreciseSecurityUnorderedSet, Beam::Threading::Mutex>;
      MarketSubscriptions<OrderImbalance> m_orderImbalanceSubscriptions;
      SecuritySubscriptions<BboQuote> m_bboQuoteSubscriptions;
      SecuritySubscriptions<BookQuote> m_bookQuoteSubscriptions;
      SecuritySubscriptions<MarketQuote> m_marketQuoteSubscriptions;
      SecuritySubscriptions<TimeAndSale> m_timeAndSaleSubscriptions;
      RealTimeMarketSubscriptionSet m_orderImbalanceRealTimeSubscriptions;
      RealTimeSecuritySubscriptionSet m_bboQuoteRealTimeSubscriptions;
      RealTimeSecuritySubscriptionSet m_bookQuoteRealTimeSubscriptions;
      RealTimeSecuritySubscriptionSet m_marketQuoteRealTimeSubscriptions;
      RealTimeSecuritySubscriptionSet m_timeAndSaleRealTimeSubscriptions;
      Beam::SynchronizedUnorderedSet<Security> m_primarySecurities;
      Beam::ResourcePool<MarketDataClient, MarketDataClientBuilder>
        m_marketDataClients;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      EntitlementDatabase m_entitlementDatabase;
      Beam::IO::OpenState m_openState;
      std::vector<std::unique_ptr<RealTimeQueryEntry>> m_realTimeQueryEntries;

      MarketDataRelayServlet(const MarketDataRelayServlet&) = delete;
      MarketDataRelayServlet& operator =(
        const MarketDataRelayServlet&) = delete;
      template<typename T>
      RealTimeQueryEntry& GetRealTimeQueryEntry(const T& index);
      template<typename Service, typename Query, typename Subscriptions,
        typename RealTimeSubscriptions>
      void HandleQueryRequest(
        Beam::Services::RequestToken<ServiceProtocolClient, Service>& request,
        const Query& query, Subscriptions& subscriptions,
        RealTimeSubscriptions& realTimeSubscriptions);
      template<typename Subscriptions>
      void OnEndQuery(ServiceProtocolClient& client,
        const typename Subscriptions::Index& index, int id,
        Subscriptions& subscriptions);
      SecuritySnapshot OnLoadSecuritySnapshot(ServiceProtocolClient& client,
        const Security& security);
      SecurityTechnicals OnLoadSecurityTechnicals(ServiceProtocolClient& client,
        const Security& security);
      std::vector<SecurityInfo> OnQuerySecurityInfo(
        ServiceProtocolClient& client, const SecurityInfoQuery& query);
      std::vector<SecurityInfo> OnLoadSecurityInfoFromPrefix(
        ServiceProtocolClient& client, const std::string& prefix);
      template<typename Index, typename Value, typename Subscriptions>
      std::enable_if_t<!std::is_same_v<Value, SequencedBookQuote>>
        OnRealTimeUpdate(const Index& index, const Value& value,
          Subscriptions& subscriptions);
      template<typename Index, typename Value, typename Subscriptions>
      std::enable_if_t<std::is_same_v<Value, SequencedBookQuote>>
        OnRealTimeUpdate(const Index& index, const Value& value,
          Subscriptions& subscriptions);
  };

  template<typename M, typename A>
  struct MetaMarketDataRelayServlet {
    using Session = MarketDataRegistrySession;
    static constexpr auto SupportsParallelism = true;

    template<typename C>
    struct apply {
      using type = MarketDataRelayServlet<C, M, A>;
    };
  };

  template<typename C, typename M, typename A>
  MarketDataRelayServlet<C, M, A>::RealTimeQueryEntry::RealTimeQueryEntry(
    std::unique_ptr<MarketDataClient> marketDataClient)
    : m_marketDataClient(std::move(marketDataClient)) {}

  template<typename C, typename M, typename A>
  template<typename AF>
  MarketDataRelayServlet<C, M, A>::MarketDataRelayServlet(
      boost::posix_time::time_duration clientTimeout,
      MarketDataClientBuilder marketDataClientBuilder,
      std::size_t minMarketDataClients, std::size_t maxMarketDataClients,
      AF&& administrationClient)
      : m_marketDataClients(clientTimeout, marketDataClientBuilder,
          minMarketDataClients, maxMarketDataClients),
        m_administrationClient(std::forward<AF>(administrationClient)),
        m_entitlementDatabase(m_administrationClient->LoadEntitlements()) {
    for(auto i = std::size_t(0); i < boost::thread::hardware_concurrency();
        ++i) {
      m_realTimeQueryEntries.emplace_back(
        std::make_unique<RealTimeQueryEntry>(marketDataClientBuilder()));
    }
  }

  template<typename C, typename M, typename A>
  void MarketDataRelayServlet<C, M, A>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterMarketDataRegistryServices(Beam::Store(slots));
    RegisterMarketDataRegistryMessages(Beam::Store(slots));
    QueryOrderImbalancesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRelayServlet::HandleQueryRequest<QueryOrderImbalancesService,
        MarketWideDataQuery, MarketSubscriptions<OrderImbalance>,
        RealTimeMarketSubscriptionSet>, this, std::placeholders::_1,
      std::placeholders::_2, std::ref(m_orderImbalanceSubscriptions),
      std::ref(m_orderImbalanceRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndOrderImbalanceQueryMessage>(Store(slots),
      std::bind(&MarketDataRelayServlet::OnEndQuery<
        MarketSubscriptions<OrderImbalance>>, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3,
        std::ref(m_orderImbalanceSubscriptions)));
    QueryBboQuotesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRelayServlet::HandleQueryRequest<QueryBboQuotesService,
        SecurityMarketDataQuery, SecuritySubscriptions<BboQuote>,
        RealTimeSecuritySubscriptionSet>, this, std::placeholders::_1,
      std::placeholders::_2, std::ref(m_bboQuoteSubscriptions),
      std::ref(m_bboQuoteRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndBboQuoteQueryMessage>(Store(slots),
      std::bind(
        &MarketDataRelayServlet::OnEndQuery<SecuritySubscriptions<BboQuote>>,
        this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::ref(m_bboQuoteSubscriptions)));
    QueryBookQuotesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRelayServlet::HandleQueryRequest<QueryBookQuotesService,
        SecurityMarketDataQuery, SecuritySubscriptions<BookQuote>,
        RealTimeSecuritySubscriptionSet>, this, std::placeholders::_1,
      std::placeholders::_2, std::ref(m_bookQuoteSubscriptions),
      std::ref(m_bookQuoteRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndBookQuoteQueryMessage>(Store(slots),
      std::bind(
        &MarketDataRelayServlet::OnEndQuery<SecuritySubscriptions<BookQuote>>,
        this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::ref(m_bookQuoteSubscriptions)));
    QueryMarketQuotesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRelayServlet::HandleQueryRequest<QueryMarketQuotesService,
        SecurityMarketDataQuery, SecuritySubscriptions<MarketQuote>,
        RealTimeSecuritySubscriptionSet>, this, std::placeholders::_1,
      std::placeholders::_2, std::ref(m_marketQuoteSubscriptions),
      std::ref(m_marketQuoteRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndMarketQuoteQueryMessage>(Store(slots),
      std::bind(
        &MarketDataRelayServlet::OnEndQuery<SecuritySubscriptions<MarketQuote>>,
        this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::ref(m_marketQuoteSubscriptions)));
    QueryTimeAndSalesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRelayServlet::HandleQueryRequest<QueryTimeAndSalesService,
        SecurityMarketDataQuery, SecuritySubscriptions<TimeAndSale>,
        RealTimeSecuritySubscriptionSet>, this, std::placeholders::_1,
      std::placeholders::_2, std::ref(m_timeAndSaleSubscriptions),
      std::ref(m_timeAndSaleRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndTimeAndSaleQueryMessage>(Store(slots),
      std::bind(
        &MarketDataRelayServlet::OnEndQuery<SecuritySubscriptions<TimeAndSale>>,
        this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::ref(m_timeAndSaleSubscriptions)));
    LoadSecuritySnapshotService::AddSlot(Store(slots),
      std::bind_front(&MarketDataRelayServlet::OnLoadSecuritySnapshot, this));
    LoadSecurityTechnicalsService::AddSlot(Store(slots),
      std::bind_front(&MarketDataRelayServlet::OnLoadSecurityTechnicals, this));
    QuerySecurityInfoService::AddSlot(Store(slots),
      std::bind_front(&MarketDataRelayServlet::OnQuerySecurityInfo, this));
    LoadSecurityInfoFromPrefixService::AddSlot(Store(slots), std::bind_front(
      &MarketDataRelayServlet::OnLoadSecurityInfoFromPrefix, this));
  }

  template<typename C, typename M, typename A>
  void MarketDataRelayServlet<C, M, A>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    session.m_roles =
      m_administrationClient->LoadAccountRoles(session.GetAccount());
    auto& entitlements = m_entitlementDatabase.GetEntries();
    auto accountEntitlements =
      m_administrationClient->LoadEntitlements(session.GetAccount());
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

  template<typename C, typename M, typename A>
  void MarketDataRelayServlet<C, M, A>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_orderImbalanceSubscriptions.RemoveAll(client);
    m_bboQuoteSubscriptions.RemoveAll(client);
    m_marketQuoteSubscriptions.RemoveAll(client);
    m_bookQuoteSubscriptions.RemoveAll(client);
    m_timeAndSaleSubscriptions.RemoveAll(client);
  }

  template<typename C, typename M, typename A>
  void MarketDataRelayServlet<C, M, A>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto closeGroup = Beam::Routines::RoutineHandlerGroup();
    for(auto& entry : m_realTimeQueryEntries) {
      closeGroup.Spawn([&] {
        entry->m_tasks.Break();
        entry->m_tasks.Wait();
        entry->m_marketDataClient->Close();
      });
    }
    auto pooledClients = std::vector<
      Beam::ScopedResource<MarketDataClient, MarketDataClientBuilder>>();
    while(auto client = m_marketDataClients.TryAcquire()) {
      pooledClients.push_back(std::move(*client));
    }
    for(auto& client : pooledClients) {
      closeGroup.Spawn([&] {
        client->Close();
      });
    }
    closeGroup.Wait();
    m_openState.Close();
  }

  template<typename C, typename M, typename A>
  template<typename T>
  typename MarketDataRelayServlet<C, M, A>::RealTimeQueryEntry&
      MarketDataRelayServlet<C, M, A>::GetRealTimeQueryEntry(const T& index) {
    auto i = std::hash<T>()(index) % m_realTimeQueryEntries.size();
    return *m_realTimeQueryEntries[i];
  }

  template<typename C, typename M, typename A>
  template<typename Service, typename Query, typename Subscriptions,
    typename RealTimeSubscriptions>
  void MarketDataRelayServlet<C, M, A>::HandleQueryRequest(
      Beam::Services::RequestToken<ServiceProtocolClient, Service>& request,
      const Query& query, Subscriptions& subscriptions,
      RealTimeSubscriptions& realTimeSubscriptions) {
    using Result = typename Service::Return;
    using MarketDataType = typename Result::Type;
    auto& session = request.GetSession();
    auto result = Result();
    if(!HasEntitlement<typename MarketDataType::Value>(session, query)) {
      request.SetResult(result);
      return;
    }
    if constexpr(std::is_same_v<typename Query::Index, Security>) {
      if(query.GetIndex().GetMarket().IsEmpty()) {
        request.SetResult(result);
        return;
      }
      auto needsValidation = [&] {
        if(auto security = m_primarySecurities.FindValue(query.GetIndex())) {
          return security->GetMarket() != query.GetIndex().GetMarket();
        }
        return true;
      }();
      if(needsValidation) {
        auto client = m_marketDataClients.Acquire();
        auto info = LoadSecurityInfo(query.GetIndex(), *client);
        if(info) {
          m_primarySecurities.Update(info->m_security);
        }
        if(!info ||
            info->m_security.GetMarket() != query.GetIndex().GetMarket()) {
          request.SetResult(result);
          return;
        }
      }
    }
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
        query.GetFilter());
      result.m_queryId = subscriptions.Initialize(query.GetIndex(),
        request.GetClient(), Beam::Queries::Range::Total(), std::move(filter));
      realTimeSubscriptions.TestAndSet(query.GetIndex(), [&] {
        auto& queryEntry = GetRealTimeQueryEntry(query.GetIndex());
        auto initialValueQueue =
          std::make_shared<Beam::Queue<MarketDataType>>();
        QueryMarketDataClient(*queryEntry.m_marketDataClient,
          Beam::Queries::MakeLatestQuery(query.GetIndex()),
          Beam::ScopedQueueWriter(initialValueQueue));
        auto initialValues = std::vector<MarketDataType>();
        Beam::Flush(initialValueQueue, std::back_inserter(initialValues));
        auto initialSequence = [&] {
          if(initialValues.empty()) {
            return Beam::Queries::Sequence::First();
          } else {
            return Beam::Queries::Increment(initialValues.back().GetSequence());
          }
        }();
        auto realTimeQuery = Query();
        realTimeQuery.SetIndex(query.GetIndex());
        realTimeQuery.SetInterruptionPolicy(
          Beam::Queries::InterruptionPolicy::RECOVER_DATA);
        realTimeQuery.SetRange(initialSequence,
          Beam::Queries::Sequence::Last());
        QueryMarketDataClient(*queryEntry.m_marketDataClient, realTimeQuery,
          queryEntry.m_tasks.template GetSlot<MarketDataType>(
            [=, &subscriptions] (const auto& value) {
              OnRealTimeUpdate(query.GetIndex(), value, subscriptions);
            }));
      });
      auto queue = std::make_shared<Beam::Queue<MarketDataType>>();
      auto client = m_marketDataClients.Acquire();
      auto snapshotQuery = query;
      snapshotQuery.SetRange(query.GetRange().GetStart(),
        Beam::Queries::Sequence::Present());
      QueryMarketDataClient(*client, snapshotQuery,
        Beam::ScopedQueueWriter(queue));
      Beam::Flush(queue, std::back_inserter(result.m_snapshot));
      subscriptions.Commit(query.GetIndex(), std::move(result),
        [&] (auto&& result) {
          request.SetResult(std::forward<decltype(result)>(result));
        });
    } else {
      auto queue = std::make_shared<Beam::Queue<MarketDataType>>();
      auto client = m_marketDataClients.Acquire();
      QueryMarketDataClient(*client, query, Beam::ScopedQueueWriter(queue));
      Beam::Flush(queue, std::back_inserter(result.m_snapshot));
      request.SetResult(result);
    }
  }

  template<typename C, typename M, typename A>
  template<typename Subscriptions>
  void MarketDataRelayServlet<C, M, A>::OnEndQuery(
      ServiceProtocolClient& client, const typename Subscriptions::Index& index,
      int id, Subscriptions& subscriptions) {
    subscriptions.End(index, id);
  }

  template<typename C, typename M, typename A>
  SecuritySnapshot MarketDataRelayServlet<C, M, A>::OnLoadSecuritySnapshot(
      ServiceProtocolClient& client, const Security& security) {
    auto& session = client.GetSession();
    auto marketDataClient = m_marketDataClients.Acquire();
    auto securitySnapshot = marketDataClient->LoadSecuritySnapshot(security);
    if(!HasEntitlement(session, security.GetMarket(),
        MarketDataType::BBO_QUOTE)) {
      securitySnapshot.m_bboQuote = SequencedBboQuote();
    }
    if(!HasEntitlement(session, security.GetMarket(),
        MarketDataType::TIME_AND_SALE)) {
      securitySnapshot.m_timeAndSale = SequencedTimeAndSale();
    }
    if(!HasEntitlement(session, security.GetMarket(),
        MarketDataType::MARKET_QUOTE)) {
      securitySnapshot.m_marketQuotes.clear();
    }
    auto askEndRange = std::remove_if(securitySnapshot.m_askBook.begin(),
      securitySnapshot.m_askBook.end(), [&] (auto& bookQuote) {
      return !HasEntitlement(session,
        EntitlementKey(security.GetMarket(), bookQuote->m_market),
        MarketDataType::BOOK_QUOTE);
    });
    securitySnapshot.m_askBook.erase(askEndRange,
      securitySnapshot.m_askBook.end());
    auto bidEndRange = std::remove_if(securitySnapshot.m_bidBook.begin(),
      securitySnapshot.m_bidBook.end(), [&] (auto& bookQuote) {
      return !HasEntitlement(session,
        EntitlementKey(security.GetMarket(), bookQuote->m_market),
        MarketDataType::BOOK_QUOTE);
    });
    securitySnapshot.m_bidBook.erase(bidEndRange,
      securitySnapshot.m_bidBook.end());
    return securitySnapshot;
  }

  template<typename C, typename M, typename A>
  SecurityTechnicals MarketDataRelayServlet<C, M, A>::OnLoadSecurityTechnicals(
      ServiceProtocolClient& client, const Security& security) {
    auto marketDataClient = m_marketDataClients.Acquire();
    return marketDataClient->LoadSecurityTechnicals(security);
  }

  template<typename C, typename M, typename A>
  std::vector<SecurityInfo> MarketDataRelayServlet<C, M, A>::
      OnQuerySecurityInfo(ServiceProtocolClient& client,
        const SecurityInfoQuery& query) {
    auto marketDataClient = m_marketDataClients.Acquire();
    return marketDataClient->QuerySecurityInfo(query);
  }

  template<typename C, typename M, typename A>
  std::vector<SecurityInfo> MarketDataRelayServlet<C, M, A>::
      OnLoadSecurityInfoFromPrefix(ServiceProtocolClient& client,
        const std::string& prefix) {
    auto marketDataClient = m_marketDataClients.Acquire();
    return marketDataClient->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename C, typename M, typename A>
  template<typename Index, typename Value, typename Subscriptions>
  std::enable_if_t<!std::is_same_v<Value, SequencedBookQuote>>
      MarketDataRelayServlet<C, M, A>::OnRealTimeUpdate(const Index& index,
        const Value& value, Subscriptions& subscriptions) {
    auto indexedValue = Beam::Queries::SequencedValue(
      Beam::Queries::IndexedValue(*value, index), value.GetSequence());
    subscriptions.Publish(indexedValue, [&] (auto& clients) {
      Beam::Services::BroadcastRecordMessage<
        GetMarketDataMessageType<typename Value::Value>>(clients, indexedValue);
    });
  }

  template<typename C, typename M, typename A>
  template<typename Index, typename Value, typename Subscriptions>
  std::enable_if_t<std::is_same_v<Value, SequencedBookQuote>>
      MarketDataRelayServlet<C, M, A>::OnRealTimeUpdate(const Index& index,
        const Value& value, Subscriptions& subscriptions) {
    auto key = EntitlementKey{index.GetMarket(), value.GetValue().m_market};
    auto indexedValue = Beam::Queries::SequencedValue(
      Beam::Queries::IndexedValue(*value, index), value.GetSequence());
    subscriptions.Publish(indexedValue,
      [&] (auto& client) {
        return HasEntitlement(client.GetSession(), key,
          MarketDataType::BOOK_QUOTE);
      },
      [&] (auto& clients) {
        Beam::Services::BroadcastRecordMessage<
          GetMarketDataMessageType<typename Value::Value>>(
            clients, indexedValue);
      });
  }
}

#endif
