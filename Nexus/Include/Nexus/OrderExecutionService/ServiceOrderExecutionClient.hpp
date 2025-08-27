#ifndef NEXUS_SERVICE_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_SERVICE_ORDER_EXECUTION_CLIENT_HPP
#include <ranges>
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
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServices.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {

  /**
   * Implements an OrderExecutionClient using Beam services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceOrderExecutionClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a ServiceOrderExecutionClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ServiceOrderExecutionClient(BF&& client_builder);
      ~ServiceOrderExecutionClient();
      std::shared_ptr<Order> load_order(OrderId id);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderRecord> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<OrderRecord> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrder> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedExecutionReport> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<ExecutionReport> queue);
      std::shared_ptr<Order> submit(const OrderFields& fields);
      void cancel(const std::shared_ptr<Order>& order);
      void cancel(const Order& order);
      void update(OrderId id, const ExecutionReport& report);
      void close();

    private:
      template<typename Value, typename Query, typename QueryService,
        typename EndQueryMessage>
      using QueryClientPublisher = Beam::Queries::QueryClientPublisher<
        Value, Query, EvaluatorTranslator,
        Beam::Services::ServiceProtocolClientHandler<B>, QueryService,
        EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_client_handler;
      QueryClientPublisher<OrderRecord, AccountQuery,
        QueryOrderSubmissionsService, EndOrderSubmissionQueryMessage>
          m_order_submission_publisher;
      QueryClientPublisher<ExecutionReport, AccountQuery,
        QueryExecutionReportsService, EndExecutionReportQueryMessage>
          m_execution_report_publisher;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      Beam::SynchronizedUnorderedSet<Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_real_time_subscriptions;
      Beam::SynchronizedVector<ExecutionReport> m_execution_report_log;
      Beam::IO::OpenState m_open_state;

      ServiceOrderExecutionClient(const ServiceOrderExecutionClient&) = delete;
      ServiceOrderExecutionClient& operator =(
        const ServiceOrderExecutionClient&) = delete;
      std::shared_ptr<PrimitiveOrder> load(const OrderRecord& record);
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void recover_orders(ServiceProtocolClient& client);
      void on_order_update(
        ServiceProtocolClient& sender, const ExecutionReport& report);
  };

  template<typename B>
  template<typename BF>
  ServiceOrderExecutionClient<B>::ServiceOrderExecutionClient(
      BF&& client_builder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_client_handler(std::forward<BF>(client_builder), std::bind_front(
              &ServiceOrderExecutionClient::on_reconnect, this)),
            m_order_submission_publisher(Beam::Ref(m_client_handler)),
            m_execution_report_publisher(Beam::Ref(m_client_handler)) {
    RegisterQueryTypes(Beam::Store(m_client_handler.GetSlots().GetRegistry()));
    RegisterOrderExecutionServices(Beam::Store(m_client_handler.GetSlots()));
    RegisterOrderExecutionMessages(Beam::Store(m_client_handler.GetSlots()));
    m_order_submission_publisher.
      template AddMessageHandler<OrderSubmissionMessage>();
    m_execution_report_publisher.
      template AddMessageHandler<ExecutionReportMessage>();
    Beam::Services::AddMessageSlot<OrderUpdateMessage>(
      Beam::Store(m_client_handler.GetSlots()),
      std::bind_front(&ServiceOrderExecutionClient::on_order_update, this));
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the order execution server."));
  }

  template<typename B>
  ServiceOrderExecutionClient<B>::~ServiceOrderExecutionClient() {
    close();
  }

  template<typename B>
  std::shared_ptr<Order>
      ServiceOrderExecutionClient<B>::load_order(OrderId id) {
    return Beam::Services::ServiceOrThrowWithNested(
      [&] () -> std::shared_ptr<Order> {
        auto client = m_client_handler.GetClient();
        if(auto record =
            client->template SendRequest<LoadOrderByIdService>(id)) {
          return load(**record);
        }
        return nullptr;
      }, "Failed to load order: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderRecord> queue) {
    m_order_submission_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(
      const AccountQuery& query, Beam::ScopedQueueWriter<OrderRecord> queue) {
    m_order_submission_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    m_order_submission_publisher.SubmitQuery(query,
      Beam::MakeConverterQueueWriter<SequencedOrderRecord>(std::move(queue),
        [this] (const auto& record) {
          return Beam::Queries::SequencedValue(std::static_pointer_cast<Order>(
            load(record)), record.GetSequence());
        }));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    m_order_submission_publisher.SubmitQuery(query,
      Beam::MakeConverterQueueWriter<SequencedOrderRecord>(std::move(queue),
        [this] (const auto& record) {
          return std::static_pointer_cast<Order>(load(record));
        }));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedExecutionReport> queue) {
    m_execution_report_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    m_execution_report_publisher.SubmitQuery(query, std::move(queue));
  }

  template<typename B>
  std::shared_ptr<Order>
      ServiceOrderExecutionClient<B>::submit(const OrderFields& fields) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      m_real_time_subscriptions.TestAndSet(fields.m_account, [&] {
        client->template SendRequest<QueryOrderSubmissionsService>(
          Beam::Queries::MakeRealTimeQuery(fields.m_account));
      });
      auto info = client->template SendRequest<NewOrderSingleService>(fields);
      auto record =
        Beam::Queries::SequencedValue(Beam::Queries::IndexedValue(OrderRecord(
          std::move(**info), {}), info->GetIndex()), info.GetSequence());
      auto order = load(**record);
      m_order_submission_publisher.Publish(record);
      return order;
    }, "Failed to submit order: " + boost::lexical_cast<std::string>(fields));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::cancel(
      const std::shared_ptr<Order>& order) {
    cancel(*order);
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::cancel(const Order& order) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      Beam::Services::SendRecordMessage<CancelOrderMessage>(
        *client, order.get_info().m_id);
    }, "Failed to cancel order: " +
      boost::lexical_cast<std::string>(order.get_info().m_id));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::update(
      OrderId id, const ExecutionReport& report) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      client->template SendRequest<UpdateOrderService>(id, report);
    }, "Failed to update order: " + boost::lexical_cast<std::string>(id) +
      ", " + boost::lexical_cast<std::string>(report));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_client_handler.Close();
    m_order_submission_publisher.Break();
    m_execution_report_publisher.Break();
    m_open_state.Close();
  }

  template<typename B>
  std::shared_ptr<PrimitiveOrder> ServiceOrderExecutionClient<B>::load(
      const OrderRecord& record) {
    return m_orders.GetOrInsert(record.m_info.m_id, [&] {
      auto complete_record = record;
      m_execution_report_log.With([&] (auto& log) {
        auto i = std::remove_if(log.begin(), log.end(), [&] (auto& report) {
          if(report.m_id == complete_record.m_info.m_id &&
              (complete_record.m_execution_reports.empty() ||
              report.m_sequence ==
              complete_record.m_execution_reports.back().m_sequence + 1)) {
            complete_record.m_execution_reports.push_back(std::move(report));
            return true;
          }
          return false;
        });
        log.erase(i, log.end());
      });
      return std::make_shared<PrimitiveOrder>(std::move(complete_record));
    });
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    recover_orders(*client);
    m_real_time_subscriptions.With([&] (const auto& subscriptions) {
      for(auto& subscription : subscriptions) {
        client->template SendRequest<QueryOrderSubmissionsService>(
          Beam::Queries::MakeRealTimeQuery(subscription));
      }
    });
    m_order_submission_publisher.Recover(*client);
    m_execution_report_publisher.Recover(*client);
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::recover_orders(
      ServiceProtocolClient& client) {
    auto entries = std::unordered_map<
      Beam::ServiceLocator::DirectoryEntry, std::vector<OrderId>>();
    m_orders.With([&] (const auto& orders) {
      for(auto& order : orders | std::views::values) {
        order->with([&] (auto status, const auto& reports) {
          if(!is_terminal(status)) {
            entries[order->get_info().m_fields.m_account].push_back(
              order->get_info().m_id);
          }
        });
      }
    });
    for(auto& entry : entries) {
      auto id_expressions = std::vector<Beam::Queries::Expression>();
      for(auto& id : entry.second) {
        auto parameter_expression =
          Beam::Queries::ParameterExpression(0, OrderInfoType());
        auto member_expression = Beam::Queries::MemberAccessExpression(
          "order_id", Beam::Queries::IdType(), parameter_expression);
        auto id_expression = Beam::Queries::ConstantExpression(id);
        auto equals_expression =
          Beam::Queries::MakeEqualsExpression(member_expression, id_expression);
        id_expressions.push_back(equals_expression);
      }
      auto filter = Beam::Queries::MakeOrExpression(
        id_expressions.begin(), id_expressions.end());
      auto query = AccountQuery();
      query.SetIndex(entry.first);
      query.SetRange(
        Beam::Queries::Sequence::First(), Beam::Queries::Sequence::Present());
      query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
      query.SetFilter(filter);
      auto result =
        client.template SendRequest<QueryOrderSubmissionsService>(query);
      for(auto& record : result.m_snapshot) {
        auto order = m_orders.Get(record->m_info.m_id);
        order->with([&] (auto status, const auto& reports) {
          for(auto& report : record->m_execution_reports) {
            if(report.m_sequence > reports.back().m_sequence) {
              order->update(report);
            }
          }
        });
      }
    }
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::on_order_update(
      ServiceProtocolClient& sender, const ExecutionReport& report) {
    if(auto order = m_orders.FindValue(report.m_id)) {
      (*order)->update(report);
    } else {
      m_execution_report_log.With([&] (auto& log) {
        auto i = Beam::LinearLowerBound(log.begin(), log.end(), report,
          [] (const auto& lhs, const auto& rhs) {
            return lhs.m_sequence < rhs.m_sequence;
          });
        if(i == log.end() || i->m_sequence != report.m_sequence) {
          log.insert(i, report);
        }
      });
    }
  }
}

#endif
