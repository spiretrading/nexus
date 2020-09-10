#ifndef NEXUS_ORDER_EXECUTION_SERVLET_HPP
#define NEXUS_ORDER_EXECUTION_SERVLET_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <boost/functional/factory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Accounting/ShortingModel.hpp"
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

  /**
   * Implements the servlet handling order submissions and cancellations.
   * @param <C> The container instantiating this servlet.
   * @param <T> The type of TimeClient used for timestamps.
   * @param <S> The type of ServiceLocatorClient used.
   * @param <U> The type of UidClient generating unique order ids.
   * @param <A> The type of AdministrationClient to use.
   * @param <O> The OrderExecutionDriver submitting and receiving the execution
   *        messaging.
   * @param <D> The type of OrderExecutionDataStore to use.
   */
  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  class OrderExecutionServlet : private boost::noncopyable {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of TimeClient used for timestamps. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /** The type of ServiceLocatorClient used. */
      using ServiceLocatorClient = Beam::GetTryDereferenceType<S>;

      /** The type of UidClient generating unique order ids. */
      using UidClient = Beam::GetTryDereferenceType<U>;

      /** The type of AdministrationClient used to check permissions. */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /**
       * The OrderExecutionDriver submitting and receiving the execution
       * messaging.
       */
      using OrderExecutionDriver = Beam::GetTryDereferenceType<O>;

      /** The type of OrderExecutionDataStore used. */
      using OrderExecutionDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs an OrderExecutionServlet.
       * @param sessionStartTime The time when the current trading session
       *        started.
       * @param marketDatabase The MarketDatabase to use.
       * @param timeClient Initializes the TimeClient.
       * @param serviceLocatorClient Initializes the ServiceLocatorClient.
       * @param uidClient Initializes the UidClient.
       * @param administrationClient Initializes the AdministrationClient.
       * @param driver Initializes the OrderExecutionDriver.
       * @param dataStore Initializes the OrderExecutionDataStore.
       */
      template<typename TF, typename SF, typename UF, typename AF, typename OF,
        typename DF>
      OrderExecutionServlet(boost::posix_time::ptime sessionStartTime,
        MarketDatabase marketDatabase, DestinationDatabase destinationDatabase,
        TF&& timeClient, SF&& serviceLocatorClient, UF&& uidClient,
        AF&& administrationClient, OF&& driver, DF&& dataStore);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      using SyncShortingModel = Beam::Threading::Sync<
        Accounting::ShortingModel>;
      boost::posix_time::ptime m_sessionStartTime;
      MarketDatabase m_marketDatabase;
      DestinationDatabase m_destinationDatabase;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      Beam::GetOptionalLocalPtr<S> m_serviceLocatorClient;
      Beam::GetOptionalLocalPtr<U> m_uidClient;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<O> m_driver;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
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
        std::shared_ptr<SyncShortingModel>> m_shortingModels;
      Beam::SynchronizedUnorderedSet<OrderId> m_liveOrders;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void RecoverTradingSession();
      void OnExecutionReport(const ExecutionReport& executionReport,
        const Beam::ServiceLocator::DirectoryEntry& account,
        SyncShortingModel& shortingModel);
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

  template<typename T, typename S, typename U, typename A, typename O,
    typename D>
  struct MetaOrderExecutionServlet {
    using Session = OrderExecutionSession;
    template<typename C>
    struct apply {
      using type = OrderExecutionServlet<C, T, S, U, A, O, D>;
    };
  };

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  template<typename TF, typename SF, typename UF, typename AF, typename OF,
    typename DF>
  OrderExecutionServlet<C, T, S, U, A, O, D>::OrderExecutionServlet(
      boost::posix_time::ptime sessionStartTime, MarketDatabase marketDatabase,
      DestinationDatabase destinationDatabase, TF&& timeClient,
      SF&& serviceLocatorClient, UF&& uidClient, AF&& administrationClient,
      OF&& driver, DF&& dataStore)
      : m_sessionStartTime(sessionStartTime),
        m_marketDatabase(std::move(marketDatabase)),
        m_destinationDatabase(std::move(destinationDatabase)),
        m_timeClient(std::forward<TF>(timeClient)),
        m_serviceLocatorClient(std::forward<SF>(serviceLocatorClient)),
        m_uidClient(std::forward<UF>(uidClient)),
        m_administrationClient(std::forward<AF>(administrationClient)),
        m_driver(std::forward<OF>(driver)),
        m_dataStore(std::forward<DF>(dataStore)) {
    try {
      auto accounts = m_serviceLocatorClient->LoadAllAccounts();
      for(auto& account : accounts) {
        m_registry.AddAccount(account);
      }
      RecoverTradingSession();
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::RegisterServices(
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

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    m_registry.AddAccount(session.GetAccount());
    session.GrantOrderExecutionPermission(session.GetAccount());
    if(m_administrationClient->CheckAdministrator(session.GetAccount())) {
      session.SetAdministrator(true);
    }
    auto tradingGroupDirectories =
      m_administrationClient->LoadManagedTradingGroups(session.GetAccount());
    for(auto& tradingGroupDirectory : tradingGroupDirectories) {
      auto tradingGroup = m_administrationClient->LoadTradingGroup(
        tradingGroupDirectory);
      for(auto& trader : tradingGroup.GetTraders()) {
        m_registry.AddAccount(trader);
        session.GrantOrderExecutionPermission(trader);
      }
    }
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_executionReportSubscriptions.RemoveAll(client);
    m_orderSubscriptions.RemoveAll(client);
    m_submissionSubscriptions.RemoveAll(client);
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_driver->Close();
    m_shortingModels.Clear();
    m_openState.Close();
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::RecoverTradingSession() {
    auto& accounts = m_serviceLocatorClient->LoadAllAccounts();
    for(auto& account : accounts) {
      auto recoveryQuery = AccountQuery();
      recoveryQuery.SetIndex(account);
      recoveryQuery.SetRange(m_sessionStartTime,
        Beam::Queries::Sequence::Last());
      recoveryQuery.SetSnapshotLimit(
        Beam::Queries::SnapshotLimit::Unlimited());
      auto& orders = m_dataStore->LoadOrderSubmissions(recoveryQuery);
      for(auto& orderRecord : orders) {
        auto& syncShortingModel = m_shortingModels.GetOrInsert(
          orderRecord->m_info.m_fields.m_account,
          boost::factory<std::shared_ptr<SyncShortingModel>>());
        syncShortingModel->With([&] (auto& shortingModel) {
          shortingModel.Submit(orderRecord->m_info.m_orderId,
            orderRecord->m_info.m_fields);
          for(auto& executionReport : orderRecord->m_executionReports) {
            shortingModel.Update(executionReport);
          }
        });
        if(!orderRecord->m_executionReports.empty() &&
            IsTerminal(orderRecord->m_executionReports.back().m_status)) {
          continue;
        }
        auto order = static_cast<const Order*>(nullptr);
        try {
          order = &m_driver->Recover(Beam::Queries::SequencedValue(
            Beam::Queries::IndexedValue(*orderRecord, account),
            orderRecord.GetSequence()));
        } catch(const std::exception&) {
          continue;
        }
        order->GetPublisher().With([&] {
          auto existingExecutionReports = 
            boost::optional<std::vector<ExecutionReport>>();
          order->GetPublisher().Monitor(m_tasks.GetSlot<ExecutionReport>(
            std::bind(&OrderExecutionServlet::OnExecutionReport, this,
            std::placeholders::_1, order->GetInfo().m_fields.m_account,
            std::ref(*syncShortingModel))),
            Beam::Store(existingExecutionReports));
          if(existingExecutionReports) {
            existingExecutionReports->erase(existingExecutionReports->begin(),
              existingExecutionReports->begin() +
              orderRecord->m_executionReports.size());
            for(auto& executionReport : *existingExecutionReports) {
              m_tasks.Push(std::bind(&OrderExecutionServlet::OnExecutionReport,
                this, executionReport, order->GetInfo().m_fields.m_account,
                std::ref(*syncShortingModel)));
            }
          }
        });
      }
    }
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::OnExecutionReport(
      const ExecutionReport& executionReport,
      const Beam::ServiceLocator::DirectoryEntry& account,
      SyncShortingModel& shortingModel) {
    Beam::Threading::With(shortingModel, [&] (auto& shortingModel) {
      shortingModel.Update(executionReport);
    });
    try {
      m_registry.Publish(Beam::Queries::IndexedValue(executionReport, account),
        [&] {
          return LoadInitialSequences(*m_dataStore, account);
        },
        [&] (auto& executionReport) {
          m_dataStore->Store(executionReport);
          m_orderSubscriptions.Publish(executionReport, [&] (auto& clients) {
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

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::
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
      auto result = OrderSubmissionQueryResult();
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      revisedQuery.GetFilter(), Beam::Ref(m_liveOrders));
    auto submissionResult = OrderSubmissionQueryResult();
    submissionResult.m_queryId = m_submissionSubscriptions.Initialize(
      revisedQuery.GetIndex(), request.GetClient(), revisedQuery.GetRange(),
      std::move(filter));
    auto executionReportResult = ExecutionReportQueryResult();
    executionReportResult.m_queryId = m_orderSubscriptions.Initialize(
      revisedQuery.GetIndex(), request.GetClient(),
      Beam::Queries::Range::Total(),
      Beam::Queries::Translate(Beam::Queries::ConstantExpression(true)));
    submissionResult.m_snapshot = m_dataStore->LoadOrderSubmissions(
      revisedQuery);
    m_submissionSubscriptions.Commit(revisedQuery.GetIndex(),
      std::move(submissionResult), [&] (auto submissionResult) {
        m_orderSubscriptions.Commit(revisedQuery.GetIndex(),
          std::move(executionReportResult), [&] (auto executionReportResult) {
            for(auto& executionReport : executionReportResult.m_snapshot) {
              auto submissionIterator = std::find_if(
                submissionResult.m_snapshot.begin(),
                submissionResult.m_snapshot.end(), [&] (auto& orderRecord) {
                return orderRecord->m_info.m_orderId == executionReport->m_id;
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

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::
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
      auto result = ExecutionReportQueryResult();
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      revisedQuery.GetFilter());
    auto result = ExecutionReportQueryResult();
    result.m_queryId = m_executionReportSubscriptions.Initialize(
      revisedQuery.GetIndex(), request.GetClient(), revisedQuery.GetRange(),
      std::move(filter));
    result.m_snapshot = m_dataStore->LoadExecutionReports(revisedQuery);
    m_executionReportSubscriptions.Commit(revisedQuery.GetIndex(),
      std::move(result), [&] (auto result) {
        request.SetResult(result);
      });
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::OnNewOrderSingleRequest(
      Beam::Services::RequestToken<ServiceProtocolClient,
      NewOrderSingleService>& request, const OrderFields& requestFields) {
    auto& session = request.GetSession();
    auto revisedFields = boost::optional<OrderFields>();
    auto fields = &requestFields;
    if(fields->m_account.m_type ==
        Beam::ServiceLocator::DirectoryEntry::Type::NONE) {
      if(!revisedFields) {
        revisedFields.emplace(*fields);
        fields = revisedFields.get_ptr();
      }
      revisedFields->m_account = session.GetAccount();
    }
    if(fields->m_destination.empty()) {
      if(!revisedFields) {
        revisedFields.emplace(*fields);
        fields = revisedFields.get_ptr();
      }
      revisedFields->m_destination =
        m_destinationDatabase.GetPreferredDestination(
        fields->m_security.GetMarket()).m_id;
    }
    if(fields->m_currency == CurrencyId::NONE) {
      if(!revisedFields) {
        revisedFields.emplace(*fields);
        fields = revisedFields.get_ptr();
      }
      revisedFields->m_currency =
        m_marketDatabase.FromCode(fields->m_security.GetMarket()).m_currency;
    }
    auto orderId = m_uidClient->LoadNextUid();
    auto shortingModel = m_shortingModels.GetOrInsert(fields->m_account,
      boost::factory<std::shared_ptr<SyncShortingModel>>());
    auto shortingFlag = Beam::Threading::With(*shortingModel,
      [&] (auto& shortingModel) {
        return shortingModel.Submit(orderId, *fields);
      });
    auto orderInfo = OrderInfo(*fields, session.GetAccount(), orderId,
      shortingFlag, m_timeClient->GetTime());
    auto order = static_cast<const Order*>(nullptr);
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
          Beam::Queries::IndexedValue(OrderRecord(**orderInfo, {}),
          orderInfo->GetIndex()), orderInfo.GetSequence());
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
      std::ref(*shortingModel))));
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::OnUpdateOrderRequest(
      ServiceProtocolClient& client, OrderId orderId,
      const ExecutionReport& executionReport) {
    auto& session = client.GetSession();
    if(!session.IsAdministrator()) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto sanitizedExecutionReport = executionReport;
    sanitizedExecutionReport.m_id = orderId;
    m_driver->Update(session, orderId, sanitizedExecutionReport);
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::OnCancelOrder(
      ServiceProtocolClient& client, OrderId orderId) {
    auto& session = client.GetSession();
    m_driver->Cancel(session, orderId);
  }
}

#endif
