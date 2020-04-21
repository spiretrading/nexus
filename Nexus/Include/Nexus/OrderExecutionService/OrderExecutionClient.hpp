#ifndef NEXUS_ORDEREXECUTIONCLIENT_HPP
#define NEXUS_ORDEREXECUTIONCLIENT_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/QueryClientPublisher.hpp>
#include <Beam/Queues/ConverterWriterQueue.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/WeakQueue.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <Beam/Utilities/SynchronizedSet.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServices.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class OrderExecutionClient
      \brief Client used to access Order execution services.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class OrderExecutionClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<
        ServiceProtocolClientBuilderType>;

      //! Constructs an OrderExecutionClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      OrderExecutionClient(ClientBuilderForward&& clientBuilder);

      ~OrderExecutionClient();

      //! Submits a query for OrderRecords.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryOrderRecords(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue);

      //! Submits a query for Order submissions.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue);

      //! Submits a query for Order submissions.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue);

      //! Submits a query for ExecutionReports.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QueryExecutionReports(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue);

      //! Submits a new single Order.
      /*!
        \param fields The OrderFields to submit.
        \return The Order that was submitted.
      */
      const Order& Submit(const OrderFields& fields);

      //! Cancels an Order.
      /*!
        \param order The Order to cancel.
      */
      void Cancel(const Order& order);

      //! Updates an Order.
      /*!
        \param orderId The id of the Order to update.
        \param executionReport The ExecutionReport containing the update.
      */
      void Update(OrderId orderId, const ExecutionReport& executionReport);

      void Open();

      void Close();

    private:
      struct OrderEntry {
        std::shared_ptr<PrimitiveOrder> m_order;
        std::vector<ExecutionReport> m_writeLog;
      };
      template<typename Value, typename Query, typename QueryService,
        typename EndQueryMessage>
      using QueryClientPublisher = Beam::Queries::QueryClientPublisher<Value,
        Query, Queries::EvaluatorTranslator,
        Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType>, QueryService, EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
      QueryClientPublisher<OrderRecord, AccountQuery,
        QueryOrderSubmissionsService, EndOrderSubmissionQueryMessage>
        m_orderSubmissionPublisher;
      QueryClientPublisher<ExecutionReport, AccountQuery,
        QueryExecutionReportsService, EndExecutionReportQueryMessage>
        m_executionReportPublisher;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      Beam::SynchronizedUnorderedSet<Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_realTimeSubscriptions;
      std::unordered_map<OrderId, OrderEntry> m_orderEntries;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_executionReportTasks;

      std::shared_ptr<PrimitiveOrder> LoadOrder(const OrderRecord& orderRecord);
      void Shutdown();
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void RecoverOrders(ServiceProtocolClient& client);
      void Publish(const ExecutionReport& executionReport);
      void OnOrderUpdate(ServiceProtocolClient& sender,
        const ExecutionReport& executionReport);
  };

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  OrderExecutionClient<ServiceProtocolClientBuilderType>::OrderExecutionClient(
      ClientBuilderForward&& clientBuilder)
      : m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)),
        m_orderSubmissionPublisher(Beam::Ref(m_clientHandler)),
        m_executionReportPublisher(Beam::Ref(m_clientHandler)) {
    m_clientHandler.SetReconnectHandler(
      std::bind(&OrderExecutionClient::OnReconnect, this,
      std::placeholders::_1));
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
      std::bind(&OrderExecutionClient::OnOrderUpdate, this,
      std::placeholders::_1, std::placeholders::_2));
  }

  template<typename ServiceProtocolClientBuilderType>
  OrderExecutionClient<ServiceProtocolClientBuilderType>::
      ~OrderExecutionClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::
      QueryOrderRecords(const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue) {
    m_orderSubmissionPublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::
      QueryOrderSubmissions(const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue) {
    auto weakQueue = Beam::MakeWeakQueue(queue);
    std::shared_ptr<Beam::QueueWriter<SequencedOrderRecord>> conversionQueue =
      Beam::MakeConverterWriterQueue<SequencedOrderRecord>(weakQueue,
      [=] (const SequencedOrderRecord& orderRecord) {
        auto sequence = orderRecord.GetSequence();
        auto order = LoadOrder(orderRecord).get();
        return Beam::Queries::SequencedValue(std::move(order),
          std::move(sequence));
      });
    m_orderSubmissionPublisher.SubmitQuery(query, conversionQueue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::
      QueryOrderSubmissions(const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue) {
    auto weakQueue = Beam::MakeWeakQueue(queue);
    std::shared_ptr<Beam::QueueWriter<SequencedOrderRecord>> conversionQueue =
      Beam::MakeConverterWriterQueue<SequencedOrderRecord>(weakQueue,
      [=] (const SequencedOrderRecord& orderRecord) {
        return LoadOrder(orderRecord).get();
      });
    m_orderSubmissionPublisher.SubmitQuery(query, conversionQueue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::
      QueryExecutionReports(const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue) {
    m_executionReportPublisher.SubmitQuery(query, queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  const Order& OrderExecutionClient<ServiceProtocolClientBuilderType>::Submit(
      const OrderFields& fields) {
    auto client = m_clientHandler.GetClient();
    if(!m_realTimeSubscriptions.Contains(fields.m_account)) {
      m_realTimeSubscriptions.With(
        [&] (auto& realTimeSubscriptions) {
          if(Beam::Contains(realTimeSubscriptions, fields.m_account)) {
            return;
          }
          AccountQuery realTimeQuery;
          realTimeQuery.SetIndex(fields.m_account);
          realTimeQuery.SetRange(Beam::Queries::Range::RealTime());
          client->template SendRequest<QueryOrderSubmissionsService>(
            realTimeQuery);
          realTimeSubscriptions.insert(fields.m_account);
        });
    }
    auto orderInfo = client->template SendRequest<NewOrderSingleService>(
      fields);
    auto orderRecord = Beam::Queries::SequencedValue(
      Beam::Queries::IndexedValue(OrderRecord{std::move(**orderInfo), {}},
      orderInfo->GetIndex()), orderInfo.GetSequence());
    auto order = LoadOrder(**orderRecord);
    m_orderSubmissionPublisher.Publish(orderRecord);
    return *order;
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::Cancel(
      const Order& order) {
    auto client = m_clientHandler.GetClient();
    Beam::Services::SendRecordMessage<CancelOrderMessage>(*client,
      order.GetInfo().m_orderId);
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::Update(
      OrderId orderId, const ExecutionReport& executionReport) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<UpdateOrderService>(orderId, executionReport);
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  std::shared_ptr<PrimitiveOrder> OrderExecutionClient<
      ServiceProtocolClientBuilderType>::LoadOrder(
      const OrderRecord& orderRecord) {
    return m_orders.GetOrInsert(orderRecord.m_info.m_orderId,
      [&] {
        auto order = std::make_shared<PrimitiveOrder>(orderRecord);
        m_executionReportTasks.Push(
          [=] {
            auto& orderEntry = m_orderEntries[orderRecord.m_info.m_orderId];
            orderEntry.m_order = order;
            if(orderEntry.m_writeLog.empty() ||
                orderEntry.m_writeLog.front().m_sequence != 0) {
              return;
            }
            orderEntry.m_order->With(
              [&] (OrderStatus orderStatus,
                  const std::vector<ExecutionReport>& executionReports) {
                while(!orderEntry.m_writeLog.empty() &&
                    (executionReports.empty() ||
                    orderEntry.m_writeLog.front().m_sequence ==
                    executionReports.back().m_sequence + 1)) {
                  orderEntry.m_order->Update(orderEntry.m_writeLog.front());
                  orderEntry.m_writeLog.erase(orderEntry.m_writeLog.begin());
                }
              });
          });
        for(const auto& executionReport : orderRecord.m_executionReports) {
          Publish(executionReport);
        }
        return order;
      });
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    RecoverOrders(*client);
    m_orderSubmissionPublisher.Recover(*client);
    m_executionReportPublisher.Recover(*client);
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::RecoverOrders(
      ServiceProtocolClient& client) {
    std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
      std::vector<OrderId>> orderEntries;
    m_orders.With(
      [&] (const std::unordered_map<OrderId, std::shared_ptr<PrimitiveOrder>>&
          orders) {
        for(const auto& order : orders | boost::adaptors::map_values) {
          order->With(
            [&] (OrderStatus status,
                const std::vector<ExecutionReport>& executionReports) {
              if(!IsTerminal(status)) {
                orderEntries[order->GetInfo().m_fields.m_account].push_back(
                  order->GetInfo().m_orderId);
              }
            });
        }
      });
    for(const auto& orderEntry : orderEntries) {
      std::vector<Beam::Queries::Expression> orderIdExpressions;
      for(const auto& orderId : orderEntry.second) {
        auto parameterExpression = Beam::Queries::ParameterExpression(
          0, Queries::OrderInfoType());
        auto memberExpression = Beam::Queries::MemberAccessExpression(
          "order_id", Beam::Queries::IdType(), parameterExpression);
        auto orderIdExpression = Beam::Queries::ConstantExpression(orderId);
        auto equalsExpression = Beam::Queries::MakeEqualsExpression(
          memberExpression, orderIdExpression);
        orderIdExpressions.push_back(equalsExpression);
      }
      auto filter = Beam::Queries::MakeOrExpression(orderIdExpressions.begin(),
        orderIdExpressions.end());
      AccountQuery query;
      query.SetIndex(orderEntry.first);
      query.SetRange(Beam::Queries::Sequence::First(),
        Beam::Queries::Sequence::Present());
      query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
      query.SetFilter(filter);
      m_executionReportTasks.Push(
        [=] {
          auto client = m_clientHandler.GetClient();
          auto queryResult = client->template SendRequest<
            QueryOrderSubmissionsService>(query);
          for(const auto& orderRecord : queryResult.m_snapshot) {
            for(const auto& executionReport : orderRecord->m_executionReports) {
              Publish(executionReport);
            }
          }
        });
    }
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::Publish(
      const ExecutionReport& executionReport) {
    m_executionReportTasks.Push(
      [=] {
        auto& orderEntry = m_orderEntries[executionReport.m_id];
        bool addToLog = false;
        if(orderEntry.m_order != nullptr) {
          orderEntry.m_order->With(
            [&] (OrderStatus orderStatus,
                const std::vector<ExecutionReport>& executionReports) {
              if((executionReports.empty() &&
                  executionReport.m_sequence == 0) ||
                  (!executionReports.empty() && executionReport.m_sequence ==
                  executionReports.back().m_sequence + 1)) {
                orderEntry.m_order->Update(executionReport);
                while(!orderEntry.m_writeLog.empty() &&
                    orderEntry.m_writeLog.front().m_sequence <=
                    executionReport.m_sequence) {
                  orderEntry.m_writeLog.erase(orderEntry.m_writeLog.begin());
                }
                while(!orderEntry.m_writeLog.empty() &&
                    orderEntry.m_writeLog.front().m_sequence ==
                    executionReports.back().m_sequence + 1) {
                  orderEntry.m_order->Update(orderEntry.m_writeLog.front());
                  orderEntry.m_writeLog.erase(orderEntry.m_writeLog.begin());
                }
              } else if(!executionReports.empty() &&
                  executionReport.m_sequence >
                  executionReports.back().m_sequence) {
                addToLog = true;
              }
            });
        } else {
          addToLog = true;
        }
        if(addToLog) {
          auto insertIterator = Beam::LinearLowerBound(
            orderEntry.m_writeLog.begin(), orderEntry.m_writeLog.end(),
            executionReport,
            [] (const ExecutionReport& lhs, const ExecutionReport& rhs) {
              return lhs.m_sequence < rhs.m_sequence;
            });
          if(insertIterator == orderEntry.m_writeLog.end() ||
              insertIterator->m_sequence != executionReport.m_sequence) {
            orderEntry.m_writeLog.insert(insertIterator, executionReport);
          }
        }
      });
  }

  template<typename ServiceProtocolClientBuilderType>
  void OrderExecutionClient<ServiceProtocolClientBuilderType>::
      OnOrderUpdate(ServiceProtocolClient& sender,
      const ExecutionReport& executionReport) {
    Publish(executionReport);
  }
}
}

#endif
