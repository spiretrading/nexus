#ifndef NEXUS_MARKET_DATA_RELAY_SERVLET_HPP
#define NEXUS_MARKET_DATA_RELAY_SERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include <Beam/Utilities/ResourcePool.hpp>
#include <Beam/Utilities/SynchronizedSet.hpp>
#include <boost/noncopyable.hpp>
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

  /** Implements a relay servlet for querying market data.
      \tparam ContainerType The container instantiating this servlet.
      \tparam MarketDataClientType The type of MarketDataClient connected to
              the source providing market data queries.
      \tparam AdministrationClientType The type of AdministrationClient to use.
   */
  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  class MarketDataRelayServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of MarketDataClient connected to the source providing market
      //! data queries.
      using MarketDataClient = Beam::GetTryDereferenceType<
        MarketDataClientType>;

      //! The type of AdministrationClient to use.
      using AdministrationClient = Beam::GetTryDereferenceType<
        AdministrationClientType>;

      //! The type of function used to builds MarketDataClients.
      using MarketDataClientBuilder =
        typename Beam::ResourcePool<MarketDataClient>::ObjectBuilder;

      //! Constructs a MarketDataRelayServlet.
      /*!
        \param entitlementDatabase The database of all market data entitlements.
        \param clientTimeout The amount of time to wait before building another
               MarketDataClient.
        \param marketDataClientBuilder Builds MarketDataClients used to
               distribute queries.
        \param minMarketDataClients The minimum number of MarketDataClients to
               pool.
        \param maxMarketDataClients The maximum number of MarketDataClients to
               pool.
        \param administrationClient Used to check for entitlements.
        \param timerThreadPool The thread pool used for timed operations.
      */
      template<typename AdministrationClientForward>
      MarketDataRelayServlet(const EntitlementDatabase& entitlementDatabase,
        const boost::posix_time::time_duration& clientTimeout,
        const MarketDataClientBuilder& marketDataClientBuilder,
        std::size_t minMarketDataClients, std::size_t maxMarketDataClients,
        AdministrationClientForward&& administrationClient,
        Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

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
      using SecuritySubscriptions = Beam::Queries::IndexedSubscriptions<
        T, Security, ServiceProtocolClient>;
      template<typename T>
      using RealTimeSubscriptionSet = Beam::SynchronizedUnorderedSet<
        T, Beam::Threading::Mutex>;
      MarketSubscriptions<OrderImbalance> m_orderImbalanceSubscriptions;
      SecuritySubscriptions<BboQuote> m_bboQuoteSubscriptions;
      SecuritySubscriptions<BookQuote> m_bookQuoteSubscriptions;
      SecuritySubscriptions<MarketQuote> m_marketQuoteSubscriptions;
      SecuritySubscriptions<TimeAndSale> m_timeAndSaleSubscriptions;
      RealTimeSubscriptionSet<MarketCode> m_orderImbalanceRealTimeSubscriptions;
      RealTimeSubscriptionSet<Security> m_bboQuoteRealTimeSubscriptions;
      RealTimeSubscriptionSet<Security> m_bookQuoteRealTimeSubscriptions;
      RealTimeSubscriptionSet<Security> m_marketQuoteRealTimeSubscriptions;
      RealTimeSubscriptionSet<Security> m_timeAndSaleRealTimeSubscriptions;
      EntitlementDatabase m_entitlementDatabase;
      Beam::ResourcePool<MarketDataClient> m_marketDataClients;
      Beam::GetOptionalLocalPtr<AdministrationClientType>
        m_administrationClient;
      Beam::IO::OpenState m_openState;
      std::vector<std::unique_ptr<RealTimeQueryEntry>> m_realTimeQueryEntries;

      void Shutdown();
      template<typename T>
      RealTimeQueryEntry& GetRealTimeQueryEntry(const T& index);
      template<typename Service, typename Query, typename Subscriptions,
        typename RealTimeSubscriptions>
      void HandleQueryRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, Service>& request, const Query& query,
        Subscriptions& subscriptions,
        RealTimeSubscriptions& realTimeSubscriptions);
      template<typename Subscriptions>
      void OnEndQuery(ServiceProtocolClient& client,
        const typename Subscriptions::Index& index, int id,
        Subscriptions& subscriptions);
      SecuritySnapshot OnLoadSecuritySnapshot(ServiceProtocolClient& client,
        const Security& security);
      SecurityTechnicals OnLoadSecurityTechnicals(ServiceProtocolClient& client,
        const Security& security);
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

  template<typename MarketDataClientType, typename AdministrationClientType>
  struct MetaMarketDataRelayServlet {
    using Session = MarketDataRegistrySession;
    static constexpr auto SupportsParallelism = true;

    template<typename ContainerType>
    struct apply {
      using type = MarketDataRelayServlet<ContainerType, MarketDataClientType,
        AdministrationClientType>;
    };
  };

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::RealTimeQueryEntry::RealTimeQueryEntry(
      std::unique_ptr<MarketDataClient> marketDataClient)
      : m_marketDataClient(std::move(marketDataClient)) {}

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  template<typename AdministrationClientForward>
  MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::MarketDataRelayServlet(
      const EntitlementDatabase& entitlementDatabase,
      const boost::posix_time::time_duration& clientTimeout,
      const MarketDataClientBuilder& marketDataClientBuilder,
      std::size_t minMarketDataClients, std::size_t maxMarketDataClients,
      AdministrationClientForward&& administrationClient,
      Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool)
      : m_entitlementDatabase(entitlementDatabase),
        m_marketDataClients(clientTimeout, marketDataClientBuilder,
          Beam::Ref(timerThreadPool), minMarketDataClients,
          maxMarketDataClients),
        m_administrationClient(std::forward<AdministrationClientForward>(
          administrationClient)) {
    for(std::size_t i = 0; i < boost::thread::hardware_concurrency(); ++i) {
      m_realTimeQueryEntries.emplace_back(
        std::make_unique<RealTimeQueryEntry>(marketDataClientBuilder()));
    }
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterMarketDataRegistryServices(Beam::Store(slots));
    RegisterMarketDataRegistryMessages(Beam::Store(slots));
    QueryOrderImbalancesService::AddRequestSlot(Store(slots),
      std::bind(&MarketDataRelayServlet::HandleQueryRequest<
      QueryOrderImbalancesService, MarketWideDataQuery,
      MarketSubscriptions<OrderImbalance>, RealTimeSubscriptionSet<MarketCode>>,
      this, std::placeholders::_1, std::placeholders::_2,
      std::ref(m_orderImbalanceSubscriptions),
      std::ref(m_orderImbalanceRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndOrderImbalanceQueryMessage>(Store(slots),
      std::bind(&MarketDataRelayServlet::OnEndQuery<
      MarketSubscriptions<OrderImbalance>>, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3,
      std::ref(m_orderImbalanceSubscriptions)));
    QueryBboQuotesService::AddRequestSlot(Store(slots),
      std::bind(&MarketDataRelayServlet::HandleQueryRequest<
      QueryBboQuotesService, SecurityMarketDataQuery,
      SecuritySubscriptions<BboQuote>, RealTimeSubscriptionSet<Security>>, this,
      std::placeholders::_1, std::placeholders::_2,
      std::ref(m_bboQuoteSubscriptions),
      std::ref(m_bboQuoteRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndBboQuoteQueryMessage>(Store(slots),
      std::bind(&MarketDataRelayServlet::OnEndQuery<
      SecuritySubscriptions<BboQuote>>, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3,
      std::ref(m_bboQuoteSubscriptions)));
    QueryBookQuotesService::AddRequestSlot(Store(slots),
      std::bind(&MarketDataRelayServlet::HandleQueryRequest<
      QueryBookQuotesService, SecurityMarketDataQuery,
      SecuritySubscriptions<BookQuote>, RealTimeSubscriptionSet<Security>>,
      this, std::placeholders::_1, std::placeholders::_2,
      std::ref(m_bookQuoteSubscriptions),
      std::ref(m_bookQuoteRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndBookQuoteQueryMessage>(Store(slots),
      std::bind(&MarketDataRelayServlet::OnEndQuery<
      SecuritySubscriptions<BookQuote>>, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3,
      std::ref(m_bookQuoteSubscriptions)));
    QueryMarketQuotesService::AddRequestSlot(Store(slots),
      std::bind(&MarketDataRelayServlet::HandleQueryRequest<
      QueryMarketQuotesService, SecurityMarketDataQuery,
      SecuritySubscriptions<MarketQuote>, RealTimeSubscriptionSet<Security>>,
      this, std::placeholders::_1, std::placeholders::_2,
      std::ref(m_marketQuoteSubscriptions),
      std::ref(m_marketQuoteRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndMarketQuoteQueryMessage>(Store(slots),
      std::bind(&MarketDataRelayServlet::OnEndQuery<
      SecuritySubscriptions<MarketQuote>>, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3,
      std::ref(m_marketQuoteSubscriptions)));
    QueryTimeAndSalesService::AddRequestSlot(Store(slots),
      std::bind(&MarketDataRelayServlet::HandleQueryRequest<
      QueryTimeAndSalesService, SecurityMarketDataQuery,
      SecuritySubscriptions<TimeAndSale>, RealTimeSubscriptionSet<Security>>,
      this, std::placeholders::_1, std::placeholders::_2,
      std::ref(m_timeAndSaleSubscriptions),
      std::ref(m_timeAndSaleRealTimeSubscriptions)));
    Beam::Services::AddMessageSlot<EndTimeAndSaleQueryMessage>(Store(slots),
      std::bind(&MarketDataRelayServlet::OnEndQuery<
      SecuritySubscriptions<TimeAndSale>>, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3,
      std::ref(m_timeAndSaleSubscriptions)));
    LoadSecuritySnapshotService::AddSlot(Store(slots), std::bind(
      &MarketDataRelayServlet::OnLoadSecuritySnapshot, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadSecurityTechnicalsService::AddSlot(Store(slots), std::bind(
      &MarketDataRelayServlet::OnLoadSecurityTechnicals, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadSecurityInfoFromPrefixService::AddSlot(Store(slots), std::bind(
      &MarketDataRelayServlet::OnLoadSecurityInfoFromPrefix, this,
      std::placeholders::_1, std::placeholders::_2));
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType, 
      AdministrationClientType>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    auto roles = m_administrationClient->LoadAccountRoles(session.GetAccount());
    if(roles.Test(AdministrationService::AccountRole::SERVICE)) {
      auto& entitlements = m_entitlementDatabase.GetEntries();
      for(auto& entitlement : entitlements) {
        for(auto& applicability : entitlement.m_applicability) {
          session.GetEntitlements().GrantEntitlement(applicability.first,
            applicability.second);
        }
      }
    } else {
      auto& entitlements = m_entitlementDatabase.GetEntries();
      auto accountEntitlements = m_administrationClient->LoadEntitlements(
        session.GetAccount());
      for(auto& entitlement : entitlements) {
        auto entryIterator = std::find(accountEntitlements.begin(),
          accountEntitlements.end(), entitlement.m_groupEntry);
        if(entryIterator != accountEntitlements.end()) {
          for(auto& applicability : entitlement.m_applicability) {
            session.GetEntitlements().GrantEntitlement(applicability.first,
              applicability.second);
          }
        }
      }
    }
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_orderImbalanceSubscriptions.RemoveAll(client);
    m_bboQuoteSubscriptions.RemoveAll(client);
    m_marketQuoteSubscriptions.RemoveAll(client);
    m_bookQuoteSubscriptions.RemoveAll(client);
    m_timeAndSaleSubscriptions.RemoveAll(client);
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::Shutdown() {
    for(auto& entry : m_realTimeQueryEntries) {
      entry->m_marketDataClient->Close();
    }
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  template<typename T>
  typename MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::RealTimeQueryEntry&
      MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::GetRealTimeQueryEntry(const T& index) {
    auto i = std::hash<T>()(index) % m_realTimeQueryEntries.size();
    return *m_realTimeQueryEntries[i];
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  template<typename Service, typename Query, typename Subscriptions,
    typename RealTimeSubscriptions>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::HandleQueryRequest(
      Beam::Services::RequestToken<ServiceProtocolClient, Service>& request,
      const Query& query, Subscriptions& subscriptions,
      RealTimeSubscriptions& realTimeSubscriptions) {
    using Result = typename Service::Return;
    using MarketDataType = typename Result::Type;
    auto& session = request.GetSession();
    auto result = Result();
    if(!HasEntitlement<typename MarketDataType::Value>(
        session.GetEntitlements(), query)) {
      request.SetResult(result);
      return;
    }
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
        query.GetFilter());
      result.m_queryId = subscriptions.Initialize(query.GetIndex(),
        request.GetClient(), Beam::Queries::Range::Total(), std::move(filter));
      realTimeSubscriptions.TestAndSet(query.GetIndex(),
        [&] {
          auto& queryEntry = GetRealTimeQueryEntry(query.GetIndex());
          auto initialValueQuery = Query();
          initialValueQuery.SetIndex(query.GetIndex());
          initialValueQuery.SetRange(Beam::Queries::Sequence::First(),
            Beam::Queries::Sequence::Present());
          initialValueQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Type::TAIL, 1);
          auto initialValueQueue =
            std::make_shared<Beam::Queue<MarketDataType>>();
          QueryMarketDataClient(*queryEntry.m_marketDataClient,
            initialValueQuery, initialValueQueue);
          auto initialValues = std::vector<MarketDataType>();
          Beam::FlushQueue(initialValueQueue,
            std::back_inserter(initialValues));
          auto initialSequence = Beam::Queries::Sequence();
          if(initialValues.empty()) {
            initialSequence = Beam::Queries::Sequence::First();
          } else {
            initialSequence = Beam::Queries::Increment(
              initialValues.back().GetSequence());
          }
          auto realTimeQuery = Query();
          realTimeQuery.SetIndex(query.GetIndex());
          realTimeQuery.SetInterruptionPolicy(
            Beam::Queries::InterruptionPolicy::RECOVER_DATA);
          realTimeQuery.SetRange(initialSequence,
            Beam::Queries::Sequence::Last());
          QueryMarketDataClient(*queryEntry.m_marketDataClient, realTimeQuery,
            queryEntry.m_tasks.template GetSlot<MarketDataType>(
            std::bind(&MarketDataRelayServlet::OnRealTimeUpdate<
            typename Query::Index, MarketDataType, Subscriptions>, this,
            query.GetIndex(), std::placeholders::_1, std::ref(subscriptions))));
        });
      auto queue = std::make_shared<Beam::Queue<MarketDataType>>();
      auto client = m_marketDataClients.Acquire();
      auto snapshotQuery = query;
      snapshotQuery.SetRange(query.GetRange().GetStart(),
        Beam::Queries::Sequence::Present());
      QueryMarketDataClient(*client, snapshotQuery, queue);
      Beam::FlushQueue(queue, std::back_inserter(result.m_snapshot));
      subscriptions.Commit(query.GetIndex(), std::move(result),
        [&] (auto&& result) {
          request.SetResult(std::forward<decltype(result)>(result));
        });
    } else {
      auto queue = std::make_shared<Beam::Queue<MarketDataType>>();
      auto client = m_marketDataClients.Acquire();
      QueryMarketDataClient(*client, query, queue);
      Beam::FlushQueue(queue, std::back_inserter(result.m_snapshot));
      request.SetResult(result);
    }
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  template<typename Subscriptions>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::OnEndQuery(ServiceProtocolClient& client,
      const typename Subscriptions::Index& index, int id,
      Subscriptions& subscriptions) {
    subscriptions.End(index, id);
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  SecuritySnapshot MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::OnLoadSecuritySnapshot(
      ServiceProtocolClient& client, const Security& security) {
    auto& session = client.GetSession();
    auto marketDataClient = m_marketDataClients.Acquire();
    auto securitySnapshot = marketDataClient->LoadSecuritySnapshot(security);
    if(!session.GetEntitlements().HasEntitlement(security.GetMarket(),
        MarketDataType::BBO_QUOTE)) {
      securitySnapshot.m_bboQuote = SequencedBboQuote();
    }
    if(!session.GetEntitlements().HasEntitlement(security.GetMarket(),
        MarketDataType::TIME_AND_SALE)) {
      securitySnapshot.m_timeAndSale = SequencedTimeAndSale();
    }
    if(!session.GetEntitlements().HasEntitlement(security.GetMarket(),
        MarketDataType::MARKET_QUOTE)) {
      securitySnapshot.m_marketQuotes.clear();
    }
    auto askEndRange = std::remove_if(securitySnapshot.m_askBook.begin(),
      securitySnapshot.m_askBook.end(),
      [&] (auto& bookQuote) {
        return !session.GetEntitlements().HasEntitlement(
          EntitlementKey(security.GetMarket(), bookQuote->m_market),
          MarketDataType::BOOK_QUOTE);
      });
    securitySnapshot.m_askBook.erase(askEndRange,
      securitySnapshot.m_askBook.end());
    auto bidEndRange = std::remove_if(securitySnapshot.m_bidBook.begin(),
      securitySnapshot.m_bidBook.end(),
      [&] (auto& bookQuote) {
        return !session.GetEntitlements().HasEntitlement(
          EntitlementKey(security.GetMarket(), bookQuote->m_market),
          MarketDataType::BOOK_QUOTE);
      });
    securitySnapshot.m_bidBook.erase(bidEndRange,
      securitySnapshot.m_bidBook.end());
    return securitySnapshot;
  }

   template<typename ContainerType, typename MarketDataClientType,
     typename AdministrationClientType>
  SecurityTechnicals MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::OnLoadSecurityTechnicals(
      ServiceProtocolClient& client, const Security& security) {
    auto marketDataClient = m_marketDataClients.Acquire();
    return marketDataClient->LoadSecurityTechnicals(security);
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  std::vector<SecurityInfo> MarketDataRelayServlet<ContainerType,
      MarketDataClientType, AdministrationClientType>::
      OnLoadSecurityInfoFromPrefix(ServiceProtocolClient& client,
      const std::string& prefix) {
    auto marketDataClient = m_marketDataClients.Acquire();
    return marketDataClient->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  template<typename Index, typename Value, typename Subscriptions>
  std::enable_if_t<!std::is_same_v<Value, SequencedBookQuote>>
      MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::OnRealTimeUpdate(const Index& index,
      const Value& value, Subscriptions& subscriptions) {
    auto indexedValue = Beam::Queries::MakeSequencedValue(
      Beam::Queries::MakeIndexedValue(*value, index), value.GetSequence());
    subscriptions.Publish(indexedValue,
      [&] (auto& clients) {
        Beam::Services::BroadcastRecordMessage<
          GetMarketDataMessageType<typename Value::Value>>(
          clients, indexedValue);
      });
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType>
  template<typename Index, typename Value, typename Subscriptions>
  std::enable_if_t<std::is_same_v<Value, SequencedBookQuote>>
      MarketDataRelayServlet<ContainerType, MarketDataClientType,
      AdministrationClientType>::OnRealTimeUpdate(const Index& index,
      const Value& value, Subscriptions& subscriptions) {
    auto key = EntitlementKey{index.GetMarket(), value.GetValue().m_market};
    auto indexedValue = Beam::Queries::MakeSequencedValue(
      Beam::Queries::MakeIndexedValue(*value, index), value.GetSequence());
    subscriptions.Publish(indexedValue,
      [&] (auto& client) {
        return client.GetSession().GetEntitlements().HasEntitlement(key,
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
