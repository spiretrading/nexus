#ifndef NEXUS_ORDER_EXECUTION_SERVLET_HPP
#define NEXUS_ORDER_EXECUTION_SERVLET_HPP
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <Beam/Utilities/SynchronizedSet.hpp>
#include <boost/functional/factory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Accounting/ShortingTracker.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServices.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionRegistry.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus::OrderExecutionService {

  /*! Stores and executes Orders.
      \tparam ContainerType The container instantiating this servlet.
      \tparam TimeClientType The type of TimeClient used for timestamps.
      \tparam ServiceLocatorClientType The type of ServiceLocatorClient used.
      \tparam UidClientType The type of UidClient generating unique order ids.
      \tparam AdministrationClientType The type of AdministrationClient to use.
      \tparam OrderExecutionDriverType The OrderExecutionDriver submitting and
              receiving the execution messaging.
      \tparam OrderExecutionDataStoreType The type of OrderExecutionDataStore
              to use.
   */
  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  class OrderExecutionServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of TimeClient used for timestamps.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! The type of ServiceLocatorClient used.
      using ServiceLocatorClient = Beam::GetTryDereferenceType<
        ServiceLocatorClientType>;

      //! The type of UidClient generating unique order ids.
      using UidClient = Beam::GetTryDereferenceType<UidClientType>;

      //! The type of AdministrationClient used to check permissions.
      using AdministrationClient = Beam::GetTryDereferenceType<
        AdministrationClientType>;

      //! The OrderExecutionDriver submitting and receiving the execution
      //! messaging.
      using OrderExecutionDriver = Beam::GetTryDereferenceType<
        OrderExecutionDriverType>;

      //! The type of OrderExecutionDataStore used.
      using OrderExecutionDataStore = Beam::GetTryDereferenceType<
        OrderExecutionDataStoreType>;

      //! Constructs an OrderExecutionServlet.
      /*!
        \param sessionStartTime The time when the current trading session
               started.
        \param marketDatabase The MarketDatabase to use.
        \param timeClient Initializes the TimeClient.
        \param serviceLocatorClient Initializes the ServiceLocatorClient.
        \param uidClient Initializes the UidClient.
        \param administrationClient Initializes the AdministrationClient.
        \param driver Initializes the OrderExecutionDriver.
        \param dataStore Initializes the OrderExecutionDataStore.
      */
      template<typename TimeClientForward,
        typename ServiceLocatorClientForward, typename UidClientForward,
        typename AdministrationClientForward,
        typename OrderExecutionDriverForward,
        typename OrderExecutionDataStoreForward>
      OrderExecutionServlet(const boost::posix_time::ptime& sessionStartTime,
        const MarketDatabase& marketDatabase,
        const DestinationDatabase& destinationDatabase,
        TimeClientForward&& timeClient,
        ServiceLocatorClientForward&& serviceLocatorClient,
        UidClientForward&& uidClient,
        AdministrationClientForward&& administrationClient,
        OrderExecutionDriverForward&& driver,
        OrderExecutionDataStoreForward&& dataStore);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      using SyncShortingTracker = Beam::Threading::Sync<
        Accounting::ShortingTracker>;
      boost::posix_time::ptime m_sessionStartTime;
      MarketDatabase m_marketDatabase;
      DestinationDatabase m_destinationDatabase;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::GetOptionalLocalPtr<ServiceLocatorClientType>
        m_serviceLocatorClient;
      Beam::GetOptionalLocalPtr<UidClientType> m_uidClient;
      Beam::GetOptionalLocalPtr<AdministrationClientType>
        m_administrationClient;
      Beam::GetOptionalLocalPtr<OrderExecutionDriverType> m_driver;
      Beam::GetOptionalLocalPtr<OrderExecutionDataStoreType> m_dataStore;
      OrderSubmissionRegistry m_registry;
      Beam::Queries::IndexedSubscriptions<OrderRecord,
        Beam::ServiceLocator::DirectoryEntry, ServiceProtocolClient>
        m_submissionSubscriptions;
      Beam::Queries::IndexedSubscriptions<ExecutionReport,
        Beam::ServiceLocator::DirectoryEntry, ServiceProtocolClient>
        m_orderSubscriptions;
      Beam::Queries::IndexedSubscriptions<ExecutionReport,
        Beam::ServiceLocator::DirectoryEntry, ServiceProtocolClient>
        m_executionReportSubscriptions;
      std::vector<std::unique_ptr<PrimitiveOrder>> m_rejectedOrders;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<SyncShortingTracker>> m_shortingTrackers;
      Beam::SynchronizedUnorderedSet<OrderId> m_liveOrders;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void RecoverTradingSession();
      void Shutdown();
      void OnExecutionReport(const ExecutionReport& executionReport,
        const Beam::ServiceLocator::DirectoryEntry& account,
        SyncShortingTracker& shortingTracker);
      void OnQueryOrderSubmissionsRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryOrderSubmissionsService>& request,
        const AccountQuery& query);
      void OnQueryExecutionReportsRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryExecutionReportsService>& request,
        const AccountQuery& query);
      void OnNewOrderSingleRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, NewOrderSingleService>& request,
        const OrderFields& requestFields);
      void OnUpdateOrderRequest(ServiceProtocolClient& client, OrderId orderId,
        const ExecutionReport& executionReport);
      void OnCancelOrder(ServiceProtocolClient& client, OrderId orderId);
  };

  template<typename TimeClientType, typename ServiceLocatorClientType,
    typename UidClientType, typename AdministrationClientType,
    typename OrderExecutionDriverType, typename OrderExecutionDataStoreType>
  struct MetaOrderExecutionServlet {
    using Session = OrderExecutionSession;
    template<typename ContainerType>
    struct apply {
      using type = OrderExecutionServlet<ContainerType, TimeClientType,
        ServiceLocatorClientType, UidClientType, AdministrationClientType,
        OrderExecutionDriverType, OrderExecutionDataStoreType>;
    };
  };

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  template<typename TimeClientForward, typename ServiceLocatorClientForward,
    typename UidClientForward, typename AdministrationClientForward,
    typename OrderExecutionDriverForward,
    typename OrderExecutionDataStoreForward>
  OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      OrderExecutionServlet(const boost::posix_time::ptime& sessionStartTime,
      const MarketDatabase& marketDatabase,
      const DestinationDatabase& destinationDatabase,
      TimeClientForward&& timeClient,
      ServiceLocatorClientForward&& serviceLocatorClient,
      UidClientForward&& uidClient,
      AdministrationClientForward&& administrationClient,
      OrderExecutionDriverForward&& driver,
      OrderExecutionDataStoreForward&& dataStore)
      : m_sessionStartTime(sessionStartTime),
        m_marketDatabase(marketDatabase),
        m_destinationDatabase(destinationDatabase),
        m_timeClient(std::forward<TimeClientForward>(timeClient)),
        m_serviceLocatorClient(std::forward<ServiceLocatorClientForward>(
          serviceLocatorClient)),
        m_uidClient(std::forward<UidClientForward>(uidClient)),
        m_administrationClient(std::forward<AdministrationClientForward>(
          administrationClient)),
        m_driver(std::forward<OrderExecutionDriverForward>(driver)),
        m_dataStore(std::forward<OrderExecutionDataStoreForward>(dataStore)) {}

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterOrderExecutionServices(Beam::Store(slots));
    RegisterOrderExecutionMessages(Beam::Store(slots));
    QueryOrderSubmissionsService::AddRequestSlot(Beam::Store(slots), std::bind(
      &OrderExecutionServlet::OnQueryOrderSubmissionsRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    QueryExecutionReportsService::AddRequestSlot(Beam::Store(slots), std::bind(
      &OrderExecutionServlet::OnQueryExecutionReportsRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    NewOrderSingleService::AddRequestSlot(Beam::Store(slots), std::bind(
      &OrderExecutionServlet::OnNewOrderSingleRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    UpdateOrderService::AddSlot(Beam::Store(slots), std::bind(
      &OrderExecutionServlet::OnUpdateOrderRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Beam::Services::AddMessageSlot<CancelOrderMessage>(Beam::Store(slots),
      std::bind(&OrderExecutionServlet::OnCancelOrder, this,
      std::placeholders::_1, std::placeholders::_2));
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      HandleClientAccepted(ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    m_registry.AddAccount(session.GetAccount());
    session.GrantOrderExecutionPermission(session.GetAccount());
    if(m_administrationClient->CheckAdministrator(session.GetAccount())) {
      session.SetAdministrator(true);
    }
    auto tradingGroupDirectories =
      m_administrationClient->LoadManagedTradingGroups(session.GetAccount());
    for(const auto& tradingGroupDirectory : tradingGroupDirectories) {
      auto tradingGroup = m_administrationClient->LoadTradingGroup(
        tradingGroupDirectory);
      for(const auto& trader : tradingGroup.GetTraders()) {
        m_registry.AddAccount(trader);
        session.GrantOrderExecutionPermission(trader);
      }
    }
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      HandleClientClosed(ServiceProtocolClient& client) {
    m_executionReportSubscriptions.RemoveAll(client);
    m_orderSubscriptions.RemoveAll(client);
    m_submissionSubscriptions.RemoveAll(client);
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_timeClient->Open();
      m_dataStore->Open();
      m_uidClient->Open();
      m_administrationClient->Open();
      m_driver->Open();
      const auto& accounts = m_serviceLocatorClient->LoadAllAccounts();
      for(const auto& account : accounts) {
        m_registry.AddAccount(account);
      }
      RecoverTradingSession();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      RecoverTradingSession() {
    const auto& accounts = m_serviceLocatorClient->LoadAllAccounts();
    for(const auto& account : accounts) {
      AccountQuery recoveryQuery;
      recoveryQuery.SetIndex(account);
      recoveryQuery.SetRange(m_sessionStartTime,
        Beam::Queries::Sequence::Last());
      recoveryQuery.SetSnapshotLimit(
        Beam::Queries::SnapshotLimit::Unlimited());
      const auto& orders = m_dataStore->LoadOrderSubmissions(recoveryQuery);
      for(const auto& orderRecord : orders) {
        auto& syncShortingTracker = m_shortingTrackers.GetOrInsert(
          orderRecord->m_info.m_fields.m_account,
          boost::factory<std::shared_ptr<SyncShortingTracker>>());
        syncShortingTracker->With(
          [&] (auto& shortingTracker) {
            shortingTracker.Submit(orderRecord->m_info.m_orderId,
              orderRecord->m_info.m_fields);
            for(auto& executionReport : orderRecord->m_executionReports) {
              shortingTracker.Update(executionReport);
            }
          });
        if(!orderRecord->m_executionReports.empty() &&
            IsTerminal(orderRecord->m_executionReports.back().m_status)) {
          continue;
        }
        const Order* order;
        try {
          order = &m_driver->Recover(Beam::Queries::SequencedValue(
            Beam::Queries::IndexedValue(*orderRecord, account),
            orderRecord.GetSequence()));
        } catch(const std::exception&) {
          continue;
        }
        order->GetPublisher().With(
          [&] {
            boost::optional<std::vector<ExecutionReport>>
              existingExecutionReports;
            order->GetPublisher().Monitor(m_tasks.GetSlot<ExecutionReport>(
              std::bind(&OrderExecutionServlet::OnExecutionReport, this,
              std::placeholders::_1, order->GetInfo().m_fields.m_account,
              std::ref(*syncShortingTracker))),
              Beam::Store(existingExecutionReports));
            if(existingExecutionReports.is_initialized()) {
              existingExecutionReports->erase(
                existingExecutionReports->begin(),
                existingExecutionReports->begin() +
                orderRecord->m_executionReports.size());
              for(const auto& executionReport : *existingExecutionReports) {
                m_tasks.Push(
                  std::bind(&OrderExecutionServlet::OnExecutionReport, this,
                  executionReport, order->GetInfo().m_fields.m_account,
                  std::ref(*syncShortingTracker)));
              }
            }
          });
      }
    }
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::Shutdown() {
    m_dataStore->Close();
    m_driver->Close();
    m_shortingTrackers.Clear();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      OnExecutionReport(const ExecutionReport& executionReport,
      const Beam::ServiceLocator::DirectoryEntry& account,
      SyncShortingTracker& shortingTracker) {
    Beam::Threading::With(shortingTracker,
      [&] (auto& shortingTracker) {
        shortingTracker.Update(executionReport);
      });
    try {
      m_registry.Publish(
        Beam::Queries::IndexedValue(executionReport, account),
        [&] {
          return LoadInitialSequences(*m_dataStore, account);
        },
        [&] (auto& executionReport) {
          m_dataStore->Store(executionReport);
          m_orderSubscriptions.Publish(executionReport,
            [&] (auto& clients) {
              Beam::Services::BroadcastRecordMessage<OrderUpdateMessage>(
                clients, **executionReport);
            });
          m_executionReportSubscriptions.Publish(executionReport,
            [&] (auto& clients) {
              Beam::Services::BroadcastRecordMessage<ExecutionReportMessage>(
                clients, executionReport);
            });
        });
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      std::cout << "\taccount: " << Beam::Serialization::ToJson(account) <<
        "\n\texecutionReport:" <<
        Beam::Serialization::ToJson(executionReport) << std::endl;
    }
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      OnQueryOrderSubmissionsRequest(Beam::Services::RequestToken<
      ServiceProtocolClient, QueryOrderSubmissionsService>& request,
      const AccountQuery& query) {
    auto& session = request.GetSession();
    auto revisedQuery = std::move(query);
    if(revisedQuery.GetIndex().m_type ==
        Beam::ServiceLocator::DirectoryEntry::Type::NONE) {
      revisedQuery.SetIndex(session.GetAccount());
    }
    if(!session.HasOrderExecutionPermission(revisedQuery.GetIndex())) {
      OrderSubmissionQueryResult result;
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      revisedQuery.GetFilter(), Beam::Ref(m_liveOrders));
    OrderSubmissionQueryResult submissionResult;
    submissionResult.m_queryId = m_submissionSubscriptions.Initialize(
      revisedQuery.GetIndex(), request.GetClient(), revisedQuery.GetRange(),
      std::move(filter));
    ExecutionReportQueryResult executionReportResult;
    executionReportResult.m_queryId = m_orderSubscriptions.Initialize(
      revisedQuery.GetIndex(), request.GetClient(),
      Beam::Queries::Range::Total(),
      Beam::Queries::Translate(Beam::Queries::ConstantExpression(true)));
    submissionResult.m_snapshot = m_dataStore->LoadOrderSubmissions(
      revisedQuery);
    m_submissionSubscriptions.Commit(revisedQuery.GetIndex(),
      std::move(submissionResult),
      [&] (auto submissionResult) {
        m_orderSubscriptions.Commit(revisedQuery.GetIndex(),
          std::move(executionReportResult),
          [&] (auto executionReportResult) {
            for(auto& executionReport : executionReportResult.m_snapshot) {
              auto submissionIterator = std::find_if(
                submissionResult.m_snapshot.begin(),
                submissionResult.m_snapshot.end(),
                [&] (auto& orderRecord) {
                  return orderRecord->m_info.m_orderId ==
                    executionReport->m_id;
                });
              if(submissionIterator == submissionResult.m_snapshot.end()) {
                continue;
              }
              auto& submission = *submissionIterator;
              auto insertionPoint = std::lower_bound(
                submission->m_executionReports.begin(),
                submission->m_executionReports.end(), *executionReport,
                [] (auto& lhs, auto& rhs) {
                  return lhs.m_sequence < rhs.m_sequence;
                });
              if(insertionPoint == submission->m_executionReports.end() ||
                  insertionPoint->m_sequence != executionReport->m_sequence) {
                submission->m_executionReports.insert(insertionPoint,
                  *executionReport);
              }
            }
            request.SetResult(submissionResult);
          });
      });
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      OnQueryExecutionReportsRequest(Beam::Services::RequestToken<
      ServiceProtocolClient, QueryExecutionReportsService>& request,
      const AccountQuery& query) {
    auto& session = request.GetSession();
    auto revisedQuery = std::move(query);
    if(revisedQuery.GetIndex().m_type ==
        Beam::ServiceLocator::DirectoryEntry::Type::NONE) {
      revisedQuery.SetIndex(session.GetAccount());
    }
    if(!session.HasOrderExecutionPermission(revisedQuery.GetIndex())) {
      ExecutionReportQueryResult result;
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      revisedQuery.GetFilter());
    ExecutionReportQueryResult result;
    result.m_queryId = m_executionReportSubscriptions.Initialize(
      revisedQuery.GetIndex(), request.GetClient(), revisedQuery.GetRange(),
      std::move(filter));
    result.m_snapshot = m_dataStore->LoadExecutionReports(revisedQuery);
    m_executionReportSubscriptions.Commit(revisedQuery.GetIndex(),
      std::move(result),
      [&] (auto result) {
        request.SetResult(result);
      });
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      OnNewOrderSingleRequest(
      Beam::Services::RequestToken<ServiceProtocolClient,
      NewOrderSingleService>& request, const OrderFields& requestFields) {
    auto& session = request.GetSession();
    boost::optional<OrderFields> revisedFields;
    auto fields = &requestFields;
    if(fields->m_account.m_type ==
        Beam::ServiceLocator::DirectoryEntry::Type::NONE) {
      if(!revisedFields.is_initialized()) {
        revisedFields.emplace(*fields);
        fields = revisedFields.get_ptr();
      }
      revisedFields->m_account = session.GetAccount();
    }
    if(fields->m_destination.empty()) {
      if(!revisedFields.is_initialized()) {
        revisedFields.emplace(*fields);
        fields = revisedFields.get_ptr();
      }
      revisedFields->m_destination =
        m_destinationDatabase.GetPreferredDestination(
        fields->m_security.GetMarket()).m_id;
    }
    if(fields->m_currency == CurrencyId::NONE()) {
      if(!revisedFields.is_initialized()) {
        revisedFields.emplace(*fields);
        fields = revisedFields.get_ptr();
      }
      revisedFields->m_currency =
        m_marketDatabase.FromCode(fields->m_security.GetMarket()).m_currency;
    }
    auto orderId = m_uidClient->LoadNextUid();
    auto shortingTracker = m_shortingTrackers.GetOrInsert(fields->m_account,
      boost::factory<std::shared_ptr<SyncShortingTracker>>());
    auto shortingFlag = Beam::Threading::With(*shortingTracker,
      [&] (auto& shortingTracker) {
        return shortingTracker.Submit(orderId, *fields);
      });
    OrderInfo orderInfo{*fields, session.GetAccount(), orderId, shortingFlag,
      m_timeClient->GetTime()};
    const Order* order;
    if(!session.HasOrderExecutionPermission(orderInfo.m_fields.m_account)) {
      auto rejectedOrder = BuildRejectedOrder(orderInfo,
        "Insufficient permissions to execute order.");
      order = rejectedOrder.get();
      m_rejectedOrders.push_back(std::move(rejectedOrder));
    } else {
      order = &m_driver->Submit(orderInfo);
    }
    m_registry.Publish(orderInfo,
      [&] {
        return LoadInitialSequences(*m_dataStore, orderInfo.m_fields.m_account);
      },
      [&] (auto& orderInfo) {
        m_liveOrders.Insert((*orderInfo)->m_orderId);
        m_dataStore->Store(orderInfo);
        request.SetResult(orderInfo);
        auto orderRecord = Beam::Queries::SequencedValue(
          Beam::Queries::IndexedValue(
          OrderRecord{**orderInfo, {}}, orderInfo->GetIndex()),
          orderInfo.GetSequence());
        m_submissionSubscriptions.Publish(orderRecord,
          [&] (auto& client) {
            return &client != &request.GetClient();
          },
          [&] (auto& clients) {
            Beam::Services::BroadcastRecordMessage<OrderSubmissionMessage>(
              clients, orderRecord);
          });
      });
    order->GetPublisher().Monitor(m_tasks.GetSlot<ExecutionReport>(
      std::bind(&OrderExecutionServlet::OnExecutionReport, this,
      std::placeholders::_1, orderInfo.m_fields.m_account,
      std::ref(*shortingTracker))));
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::
      OnUpdateOrderRequest(ServiceProtocolClient& client, OrderId orderId,
      const ExecutionReport& executionReport) {
    auto& session = client.GetSession();
    if(!session.IsAdministrator()) {
      throw Beam::Services::ServiceRequestException{
        "Insufficient permissions."};
    }
    auto sanitizedExecutionReport = executionReport;
    sanitizedExecutionReport.m_id = orderId;
    m_driver->Update(session, orderId, sanitizedExecutionReport);
  }

  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType>
  void OrderExecutionServlet<ContainerType, TimeClientType,
      ServiceLocatorClientType, UidClientType, AdministrationClientType,
      OrderExecutionDriverType, OrderExecutionDataStoreType>::OnCancelOrder(
      ServiceProtocolClient& client, OrderId orderId) {
    auto& session = client.GetSession();
    m_driver->Cancel(session, orderId);
  }
}

#endif
