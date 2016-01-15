#ifndef NEXUS_MARKETDATARELAYSERVLET_HPP
#define NEXUS_MARKETDATARELAYSERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include <Beam/Utilities/ResourcePool.hpp>
#include <Beam/Utilities/SynchronizedSet.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataClientUtilities.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/MarketDataService/MarketDataRegistrySession.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MarketDataRelayServlet
      \brief Implements a relay servlet for querying market data.
      \tparam ContainerType The container instantiating this servlet.
      \tparam MarketDataClientType The type of MarketDataClient connected to
              the source providing market data queries.
      \tparam ServiceLocatorClientType The type of ServiceLocatorClient to use.
   */
  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  class MarketDataRelayServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of MarketDataClient connected to the source providing market
      //! data queries.
      using MarketDataClient = MarketDataClientType;

      //! The type of ServiceLocatorClient to use.
      using ServiceLocatorClient = ServiceLocatorClientType;

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
        \param serviceLocatorClient Used to check for entitlements.
        \param timerThreadPool The thread pool used for timed operations.
      */
      MarketDataRelayServlet(const EntitlementDatabase& entitlementDatabase,
        const boost::posix_time::time_duration& clientTimeout,
        const MarketDataClientBuilder& marketDataClientBuilder,
        std::size_t minMarketDataClients, std::size_t maxMarketDataClients,
        Beam::RefType<ServiceLocatorClient> serviceLocatorClient,
        Beam::RefType<Beam::Threading::TimerThreadPool> timerThreadPool);

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
      ServiceLocatorClient* m_serviceLocatorClient;
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
      typename std::enable_if<
        !std::is_same<Value, SequencedBookQuote>::value>::type OnRealTimeUpdate(
        const Index& index, const Value& value, Subscriptions& subscriptions);
      template<typename Index, typename Value, typename Subscriptions>
      typename std::enable_if<
        std::is_same<Value, SequencedBookQuote>::value>::type OnRealTimeUpdate(
        const Index& index, const Value& value, Subscriptions& subscriptions);
  };

  template<typename MarketDataClientType, typename ServiceLocatorClientType>
  struct MetaMarketDataRelayServlet {
    using Session = MarketDataRegistrySession;
    template<typename ContainerType>
    struct apply {
      using type = MarketDataRelayServlet<ContainerType, MarketDataClientType,
        ServiceLocatorClientType>;
    };
  };

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::RealTimeQueryEntry::RealTimeQueryEntry(
      std::unique_ptr<MarketDataClient> marketDataClient)
      : m_marketDataClient(std::move(marketDataClient)) {}

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::MarketDataRelayServlet(
      const EntitlementDatabase& entitlementDatabase,
      const boost::posix_time::time_duration& clientTimeout,
      const MarketDataClientBuilder& marketDataClientBuilder,
      std::size_t minMarketDataClients, std::size_t maxMarketDataClients,
      Beam::RefType<ServiceLocatorClient> serviceLocatorClient,
      Beam::RefType<Beam::Threading::TimerThreadPool> timerThreadPool)
      : m_entitlementDatabase(entitlementDatabase),
        m_marketDataClients(clientTimeout, marketDataClientBuilder,
          Beam::Ref(timerThreadPool), minMarketDataClients,
          maxMarketDataClients),
        m_serviceLocatorClient(serviceLocatorClient.Get()) {
    for(std::size_t i = 0; i < boost::thread::hardware_concurrency(); ++i) {
      m_realTimeQueryEntries.emplace_back(
        std::make_unique<RealTimeQueryEntry>(marketDataClientBuilder()));
    }
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::RegisterServices(
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
    typename ServiceLocatorClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType, 
      ServiceLocatorClientType>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    auto parents = m_serviceLocatorClient->LoadParents(session.GetAccount());
    const auto& entitlements = m_entitlementDatabase.GetEntries();
    for(const auto& entitlement : entitlements) {
      auto entryIterator = std::find(parents.begin(), parents.end(),
        entitlement.m_groupEntry);
      if(entryIterator != parents.end()) {
        for(const auto& applicability : entitlement.m_applicability) {
          session.GetEntitlements().GrantEntitlement(applicability.first,
            applicability.second);
        }
      }
    }
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_orderImbalanceSubscriptions.RemoveAll(client);
    m_bboQuoteSubscriptions.RemoveAll(client);
    m_marketQuoteSubscriptions.RemoveAll(client);
    m_bookQuoteSubscriptions.RemoveAll(client);
    m_timeAndSaleSubscriptions.RemoveAll(client);
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::Shutdown() {
    for(const auto& entry : m_realTimeQueryEntries) {
      entry->m_marketDataClient->Close();
    }
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  template<typename T>
  typename MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::RealTimeQueryEntry&
      MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::GetRealTimeQueryEntry(const T& index) {
    auto i = std::hash<T>()(index) % m_realTimeQueryEntries.size();
    return *m_realTimeQueryEntries[i];
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  template<typename Service, typename Query, typename Subscriptions,
    typename RealTimeSubscriptions>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::HandleQueryRequest(
      Beam::Services::RequestToken<ServiceProtocolClient, Service>& request,
      const Query& query, Subscriptions& subscriptions,
      RealTimeSubscriptions& realTimeSubscriptions) {
    using Result = typename Service::Return;
    using MarketDataType = typename Result::Type;
    auto& session = request.GetSession();
    Result result;
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
          Query initialValueQuery;
          initialValueQuery.SetIndex(query.GetIndex());
          initialValueQuery.SetRange(Beam::Queries::Sequence::First(),
            Beam::Queries::Sequence::Present());
          initialValueQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Type::TAIL, 1);
          auto initialValueQueue =
            std::make_shared<Beam::Queue<MarketDataType>>();
          QueryMarketDataClient<MarketDataType>(*queryEntry.m_marketDataClient,
            initialValueQuery, initialValueQueue);
          std::vector<MarketDataType> initialValues;
          Beam::FlushQueue(initialValueQueue,
            std::back_inserter(initialValues));
          Beam::Queries::Sequence initialSequence;
          if(initialValues.empty()) {
            initialSequence = Beam::Queries::Sequence::First();
          } else {
            initialSequence = Beam::Queries::Increment(
              initialValues.back().GetSequence());
          }
          Query realTimeQuery;
          realTimeQuery.SetIndex(query.GetIndex());
          realTimeQuery.SetInterruptionPolicy(
            Beam::Queries::InterruptionPolicy::RECOVER_DATA);
          realTimeQuery.SetRange(initialSequence,
            Beam::Queries::Sequence::Last());
          QueryMarketDataClient<MarketDataType>(
            *queryEntry.m_marketDataClient, realTimeQuery,
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
      QueryMarketDataClient<MarketDataType>(*client, snapshotQuery, queue);
      Beam::FlushQueue(queue, std::back_inserter(result.m_snapshot));
      subscriptions.Commit(query.GetIndex(), std::move(result),
        [&] (const Result& result) {
          request.SetResult(result);
        });
    } else {
      auto queue = std::make_shared<Beam::Queue<MarketDataType>>();
      auto client = m_marketDataClients.Acquire();
      QueryMarketDataClient<MarketDataType>(*client, query, queue);
      Beam::FlushQueue(queue, std::back_inserter(result.m_snapshot));
      request.SetResult(result);
    }
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  template<typename Subscriptions>
  void MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::OnEndQuery(ServiceProtocolClient& client,
      const typename Subscriptions::Index& index, int id,
      Subscriptions& subscriptions) {
    subscriptions.End(index, id);
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  SecuritySnapshot MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::OnLoadSecuritySnapshot(
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
      [&] (const BookQuote& bookQuote) {
        return !session.GetEntitlements().HasEntitlement(
          EntitlementKey(security.GetMarket(), bookQuote.m_market),
          MarketDataType::BOOK_QUOTE);
      });
    securitySnapshot.m_askBook.erase(askEndRange,
      securitySnapshot.m_askBook.end());
    auto bidEndRange = std::remove_if(securitySnapshot.m_bidBook.begin(),
      securitySnapshot.m_bidBook.end(),
      [&] (const BookQuote& bookQuote) {
        return !session.GetEntitlements().HasEntitlement(
          EntitlementKey(security.GetMarket(), bookQuote.m_market),
          MarketDataType::BOOK_QUOTE);
      });
    securitySnapshot.m_bidBook.erase(bidEndRange,
      securitySnapshot.m_bidBook.end());
    return securitySnapshot;
  }

   template<typename ContainerType, typename MarketDataClientType,
     typename ServiceLocatorClientType>
  SecurityTechnicals MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::OnLoadSecurityTechnicals(
      ServiceProtocolClient& client, const Security& security) {
    auto marketDataClient = m_marketDataClients.Acquire();
    return marketDataClient->LoadSecurityTechnicals(security);
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  std::vector<SecurityInfo> MarketDataRelayServlet<ContainerType,
      MarketDataClientType, ServiceLocatorClientType>::
      OnLoadSecurityInfoFromPrefix(ServiceProtocolClient& client,
      const std::string& prefix) {
    auto marketDataClient = m_marketDataClients.Acquire();
    return marketDataClient->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  template<typename Index, typename Value, typename Subscriptions>
  typename std::enable_if<!std::is_same<Value, SequencedBookQuote>::value>::type
      MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::OnRealTimeUpdate(const Index& index,
      const Value& value, Subscriptions& subscriptions) {
    auto indexedValue = Beam::Queries::MakeSequencedValue(
      Beam::Queries::MakeIndexedValue(*value, index), value.GetSequence());
    subscriptions.Publish(indexedValue,
      [&] (const std::vector<ServiceProtocolClient*>& clients) {
        Beam::Services::BroadcastRecordMessage<
          GetMarketDataMessageType<typename Value::Value>>(
          clients, indexedValue);
      });
  }

  template<typename ContainerType, typename MarketDataClientType,
    typename ServiceLocatorClientType>
  template<typename Index, typename Value, typename Subscriptions>
  typename std::enable_if<std::is_same<Value, SequencedBookQuote>::value>::type
      MarketDataRelayServlet<ContainerType, MarketDataClientType,
      ServiceLocatorClientType>::OnRealTimeUpdate(const Index& index,
      const Value& value, Subscriptions& subscriptions) {
    auto key = EntitlementKey{index.GetMarket(), value.GetValue().m_market};
    auto indexedValue = Beam::Queries::MakeSequencedValue(
      Beam::Queries::MakeIndexedValue(*value, index), value.GetSequence());
    subscriptions.Publish(indexedValue,
      [&] (const ServiceProtocolClient& client) {
        return client.GetSession().GetEntitlements().HasEntitlement(key,
          MarketDataType::BOOK_QUOTE);
      },
      [&] (const std::vector<ServiceProtocolClient*>& clients) {
        Beam::Services::BroadcastRecordMessage<
          GetMarketDataMessageType<typename Value::Value>>(
          clients, indexedValue);
      });
  }
}
}

#endif
