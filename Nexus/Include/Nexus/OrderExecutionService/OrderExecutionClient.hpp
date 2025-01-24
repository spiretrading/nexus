#ifndef NEXUS_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_ORDER_EXECUTION_CLIENT_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/QueryClientPublisher.hpp>
#include <Beam/Queues/ConverterQueueWriter.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServices.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Client used to access Order execution services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class OrderExecutionClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs an OrderExecutionClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit OrderExecutionClient(BF&& clientBuilder);

      ~OrderExecutionClient();

      /** Loads an Order by its id. */
      boost::optional<const Order&> LoadOrder(OrderId id);

      /**
       * Submits a query for SequencedOrderRecords.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryOrderRecords(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderRecord> queue);

      /**
       * Submits a query for OrderRecords.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryOrderRecords(const AccountQuery& query,
        Beam::ScopedQueueWriter<OrderRecord> queue);

      /**
       * Submits a query for SequencedOrder submissions.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryOrderSubmissions(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrder> queue);

      /**
       * Submits a query for Order submissions.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryOrderSubmissions(const AccountQuery& query,
        Beam::ScopedQueueWriter<const Order*> queue);

      /**
       * Submits a query for SequencedExecutionReports.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryExecutionReports(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedExecutionReport> queue);

      /**
       * Submits a query for ExecutionReports.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void QueryExecutionReports(const AccountQuery& query,
        Beam::ScopedQueueWriter<ExecutionReport> queue);

      /**
       * Submits a new single Order.
       * @param fields The OrderFields to submit.
       * @return The Order that was submitted.
       */
      const Order& Submit(const OrderFields& fields);

      /**
       * Cancels an Order.
       * @param order The Order to cancel.
       */
      void Cancel(const Order& order);

      /**
       * Updates an Order.
       * @param orderId The id of the Order to update.
       * @param executionReport The ExecutionReport containing the update.
       */
      void Update(OrderId orderId, const ExecutionReport& executionReport);

      void Close();

    private:
      template<typename Value, typename Query, typename QueryService,
        typename EndQueryMessage>
      using QueryClientPublisher = Beam::Queries::QueryClientPublisher<Value,
        Query, Queries::EvaluatorTranslator,
        Beam::Services::ServiceProtocolClientHandler<B>, QueryService,
        EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      QueryClientPublisher<OrderRecord, AccountQuery,
        QueryOrderSubmissionsService, EndOrderSubmissionQueryMessage>
          m_orderSubmissionPublisher;
      QueryClientPublisher<ExecutionReport, AccountQuery,
        QueryExecutionReportsService, EndExecutionReportQueryMessage>
          m_executionReportPublisher;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      Beam::SynchronizedUnorderedSet<Beam::ServiceLocator::DirectoryEntry>
        m_realTimeSubscriptions;
      Beam::SynchronizedVector<ExecutionReport> m_executionReportLog;
      Beam::IO::OpenState m_openState;

      OrderExecutionClient(const OrderExecutionClient&) = delete;
      OrderExecutionClient& operator =(const OrderExecutionClient&) = delete;
      std::shared_ptr<PrimitiveOrder> LoadOrder(const OrderRecord& orderRecord);
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void RecoverOrders(ServiceProtocolClient& client);
      void OnOrderUpdate(ServiceProtocolClient& sender,
        const ExecutionReport& executionReport);
  };

  template<typename B>
  template<typename BF>
  OrderExecutionClient<B>::OrderExecutionClient(BF&& clientBuilder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_clientHandler(std::forward<BF>(clientBuilder),
              std::bind_front(&OrderExecutionClient::OnReconnect, this)),
            m_orderSubmissionPublisher(Beam::Ref(m_clientHandler)),
            m_executionReportPublisher(Beam::Ref(m_clientHandler)) {
    Queries::RegisterQueryTypes(
      Beam::Store(m_clientHandler.GetSlots().GetRegistry()));
    RegisterOrderExecutionServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterOrderExecutionMessages(Beam::Store(m_clientHandler.GetSlots()));
    m_orderSubmissionPublisher.
      template AddMessageHandler<OrderSubmissionMessage>();
    m_executionReportPublisher.
      template AddMessageHandler<ExecutionReportMessage>();
    Beam::Services::AddMessageSlot<OrderUpdateMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind_front(&OrderExecutionClient::OnOrderUpdate, this));
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the order execution server."));
  }

  template<typename B>
  OrderExecutionClient<B>::~OrderExecutionClient() {
    Close();
  }

  template<typename B>
  boost::optional<const Order&> OrderExecutionClient<B>::LoadOrder(OrderId id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      if(auto record = client->template SendRequest<LoadOrderByIdService>(id)) {
        return boost::optional<const Order&>(
          *static_cast<const Order*>(LoadOrder(**record).get()));
      }
      return boost::optional<const Order&>();
    }, "Failed to load order: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void OrderExecutionClient<B>::QueryOrderRecords(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderRecord> queue) {
    m_orderSubmissionPublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void OrderExecutionClient<B>::QueryOrderRecords(const AccountQuery& query,
      Beam::ScopedQueueWriter<OrderRecord> queue) {
    m_orderSubmissionPublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void OrderExecutionClient<B>::QueryOrderSubmissions(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    m_orderSubmissionPublisher.SubmitQuery(query,
      Beam::MakeConverterQueueWriter<SequencedOrderRecord>(std::move(queue),
      [this] (const auto& orderRecord) {
        return Beam::Queries::SequencedValue(
          static_cast<const Order*>(LoadOrder(orderRecord).get()),
          orderRecord.GetSequence());
      }));
  }

  template<typename B>
  void OrderExecutionClient<B>::QueryOrderSubmissions(const AccountQuery& query,
      Beam::ScopedQueueWriter<const Order*> queue) {
    m_orderSubmissionPublisher.SubmitQuery(query,
      Beam::MakeConverterQueueWriter<SequencedOrderRecord>(std::move(queue),
      [this] (const auto& orderRecord) {
        return static_cast<const Order*>(LoadOrder(orderRecord).get());
      }));
  }

  template<typename B>
  void OrderExecutionClient<B>::QueryExecutionReports(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedExecutionReport> queue) {
    m_executionReportPublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void OrderExecutionClient<B>::QueryExecutionReports(const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    m_executionReportPublisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  const Order& OrderExecutionClient<B>::Submit(const OrderFields& fields) {
    return Beam::Services::ServiceOrThrowWithNested([&] () -> decltype(auto) {
      auto client = m_clientHandler.GetClient();
      m_realTimeSubscriptions.TestAndSet(fields.m_account, [&] {
        client->template SendRequest<QueryOrderSubmissionsService>(
          Beam::Queries::MakeRealTimeQuery(fields.m_account));
      });
      auto orderInfo = client->template SendRequest<NewOrderSingleService>(
        fields);
      auto orderRecord = Beam::Queries::SequencedValue(
        Beam::Queries::IndexedValue(OrderRecord(std::move(**orderInfo), {}),
        orderInfo->GetIndex()), orderInfo.GetSequence());
      auto order = LoadOrder(**orderRecord);
      m_orderSubmissionPublisher.Publish(orderRecord);
      return *order;
    }, "Failed to submit order: " + boost::lexical_cast<std::string>(fields));
  }

  template<typename B>
  void OrderExecutionClient<B>::Cancel(const Order& order) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      Beam::Services::SendRecordMessage<CancelOrderMessage>(*client,
        order.GetInfo().m_orderId);
    }, "Failed to cancel order: " +
      boost::lexical_cast<std::string>(order.GetInfo().m_orderId));
  }

  template<typename B>
  void OrderExecutionClient<B>::Update(OrderId orderId,
      const ExecutionReport& executionReport) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<UpdateOrderService>(orderId,
        executionReport);
    }, "Failed to update order: " + boost::lexical_cast<std::string>(orderId) +
      ", " + boost::lexical_cast<std::string>(executionReport));
  }

  template<typename B>
  void OrderExecutionClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_orderSubmissionPublisher.Break();
    m_executionReportPublisher.Break();
    m_openState.Close();
  }

  template<typename B>
  std::shared_ptr<PrimitiveOrder> OrderExecutionClient<B>::LoadOrder(
      const OrderRecord& orderRecord) {
    return m_orders.GetOrInsert(orderRecord.m_info.m_orderId, [&] {
      auto completeOrderRecord = orderRecord;
      m_executionReportLog.With([&] (auto& log) {
        auto i = std::remove_if(log.begin(), log.end(), [&] (auto& report) {
          if(report.m_id == completeOrderRecord.m_info.m_orderId &&
              (completeOrderRecord.m_executionReports.empty() ||
              report.m_sequence ==
              completeOrderRecord.m_executionReports.back().m_sequence + 1)) {
            completeOrderRecord.m_executionReports.push_back(std::move(report));
            return true;
          }
          return false;
        });
        log.erase(i, log.end());
      });
      return std::make_shared<PrimitiveOrder>(std::move(completeOrderRecord));
    });
  }

  template<typename B>
  void OrderExecutionClient<B>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    RecoverOrders(*client);
    m_realTimeSubscriptions.With([&] (const auto& subscriptions) {
      for(auto& subscription : subscriptions) {
        client->template SendRequest<QueryOrderSubmissionsService>(
          Beam::Queries::MakeRealTimeQuery(subscription));
      }
    });
    m_orderSubmissionPublisher.Recover(*client);
    m_executionReportPublisher.Recover(*client);
  }

  template<typename B>
  void OrderExecutionClient<B>::RecoverOrders(ServiceProtocolClient& client) {
    auto orderEntries = std::unordered_map<
      Beam::ServiceLocator::DirectoryEntry, std::vector<OrderId>>();
    m_orders.With([&] (const auto& orders) {
      for(auto& order : orders | boost::adaptors::map_values) {
        order->With([&] (auto status, const auto& reports) {
          if(!IsTerminal(status)) {
            orderEntries[order->GetInfo().m_fields.m_account].push_back(
              order->GetInfo().m_orderId);
          }
        });
      }
    });
    for(auto& orderEntry : orderEntries) {
      auto orderIdExpressions = std::vector<Beam::Queries::Expression>();
      for(auto& orderId : orderEntry.second) {
        auto parameterExpression = Beam::Queries::ParameterExpression(
          0, Queries::OrderInfoType());
        auto memberExpression = Beam::Queries::MemberAccessExpression(
          "order_id", Beam::Queries::IdType(), parameterExpression);
        auto orderIdExpression = Beam::Queries::ConstantExpression(orderId);
        auto equalsExpression = Beam::Queries::MakeEqualsExpression(
          memberExpression, orderIdExpression);
        orderIdExpressions.push_back(equalsExpression);
      }
      auto filter = Beam::Queries::MakeOrExpression(
        orderIdExpressions.begin(), orderIdExpressions.end());
      auto query = AccountQuery();
      query.SetIndex(orderEntry.first);
      query.SetRange(Beam::Queries::Sequence::First(),
        Beam::Queries::Sequence::Present());
      query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
      query.SetFilter(filter);
      auto queryResult =
        client.template SendRequest<QueryOrderSubmissionsService>(query);
      for(auto& orderRecord : queryResult.m_snapshot) {
        auto order = m_orders.Get(orderRecord->m_info.m_orderId);
        order->With([&] (auto status, const auto& reports) {
          for(auto& executionReport : orderRecord->m_executionReports) {
            if(executionReport.m_sequence > reports.back().m_sequence) {
              order->Update(executionReport);
            }
          }
        });
      }
    }
  }

  template<typename B>
  void OrderExecutionClient<B>::OnOrderUpdate(ServiceProtocolClient& sender,
      const ExecutionReport& executionReport) {
    if(auto order = m_orders.FindValue(executionReport.m_id)) {
      (*order)->Update(executionReport);
    } else {
      m_executionReportLog.With([&] (auto& log) {
        auto i = Beam::LinearLowerBound(log.begin(), log.end(), executionReport,
          [] (const auto& lhs, const auto& rhs) {
            return lhs.m_sequence < rhs.m_sequence;
          });
        if(i == log.end() || i->m_sequence != executionReport.m_sequence) {
          log.insert(i, executionReport);
        }
      });
    }
  }
}

#endif
