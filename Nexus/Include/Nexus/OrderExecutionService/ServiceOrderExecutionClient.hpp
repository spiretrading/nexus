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
      using ServiceProtocolClientBuilder = Beam::dereference_t<B>;

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
      using QueryClientPublisher = Beam::QueryClientPublisher<
        Value, Query, EvaluatorTranslator,
        Beam::ServiceProtocolClientHandler<B>, QueryService, EndQueryMessage>;
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::ServiceProtocolClientHandler<B> m_client_handler;
      QueryClientPublisher<OrderRecord, AccountQuery,
        QueryOrderSubmissionsService, EndOrderSubmissionQueryMessage>
          m_order_submission_publisher;
      QueryClientPublisher<ExecutionReport, AccountQuery,
        QueryExecutionReportsService, EndExecutionReportQueryMessage>
          m_execution_report_publisher;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      Beam::SynchronizedUnorderedSet<Beam::DirectoryEntry, Beam::Mutex>
        m_real_time_subscriptions;
      Beam::SynchronizedVector<ExecutionReport> m_execution_report_log;
      Beam::OpenState m_open_state;

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
    Nexus::register_query_types(
      Beam::out(m_client_handler.get_slots().get_registry()));
    register_order_execution_services(Beam::out(m_client_handler.get_slots()));
    register_order_execution_messages(Beam::out(m_client_handler.get_slots()));
    m_order_submission_publisher.
      template add_message_handler<OrderSubmissionMessage>();
    m_execution_report_publisher.
      template add_message_handler<ExecutionReportMessage>();
    Beam::add_message_slot<OrderUpdateMessage>(
      Beam::out(m_client_handler.get_slots()),
      std::bind_front(&ServiceOrderExecutionClient::on_order_update, this));
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to connect to the order execution server."));
  }

  template<typename B>
  ServiceOrderExecutionClient<B>::~ServiceOrderExecutionClient() {
    close();
  }

  template<typename B>
  std::shared_ptr<Order>
      ServiceOrderExecutionClient<B>::load_order(OrderId id) {
    return Beam::service_or_throw_with_nested(
      [&] () -> std::shared_ptr<Order> {
        auto client = m_client_handler.get_client();
        if(auto record =
            client->template send_request<LoadOrderByIdService>(id)) {
          return load(**record);
        }
        return nullptr;
      }, "Failed to load order: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderRecord> queue) {
    m_order_submission_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(
      const AccountQuery& query, Beam::ScopedQueueWriter<OrderRecord> queue) {
    m_order_submission_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    m_order_submission_publisher.submit(query,
      Beam::convert<SequencedOrderRecord>(std::move(queue),
        [this] (const auto& record) {
          return Beam::SequencedValue(std::static_pointer_cast<Order>(
            load(record)), record.get_sequence());
        }));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    m_order_submission_publisher.submit(query,
      Beam::convert<SequencedOrderRecord>(std::move(queue),
        [this] (const auto& record) {
          return std::static_pointer_cast<Order>(load(record));
        }));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedExecutionReport> queue) {
    m_execution_report_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    m_execution_report_publisher.submit(query, std::move(queue));
  }

  template<typename B>
  std::shared_ptr<Order>
      ServiceOrderExecutionClient<B>::submit(const OrderFields& fields) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      m_real_time_subscriptions.test_and_set(fields.m_account, [&] {
        client->template send_request<QueryOrderSubmissionsService>(
          Beam::make_real_time_query(fields.m_account));
      });
      auto info = client->template send_request<NewOrderSingleService>(fields);
      auto record = Beam::SequencedValue(Beam::IndexedValue(OrderRecord(
        std::move(**info), {}), info->get_index()), info.get_sequence());
      auto order = load(**record);
      m_order_submission_publisher.publish(record);
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
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      Beam::send_record_message<CancelOrderMessage>(
        *client, order.get_info().m_id);
    }, "Failed to cancel order: " +
      boost::lexical_cast<std::string>(order.get_info().m_id));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::update(
      OrderId id, const ExecutionReport& report) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<UpdateOrderService>(id, report);
    }, "Failed to update order: " + boost::lexical_cast<std::string>(id) +
      ", " + boost::lexical_cast<std::string>(report));
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_client_handler.close();
    m_order_submission_publisher.close();
    m_execution_report_publisher.close();
    m_open_state.close();
  }

  template<typename B>
  std::shared_ptr<PrimitiveOrder> ServiceOrderExecutionClient<B>::load(
      const OrderRecord& record) {
    return m_orders.get_or_insert(record.m_info.m_id, [&] {
      auto complete_record = record;
      m_execution_report_log.with([&] (auto& log) {
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
    m_real_time_subscriptions.for_each([&] (auto& subscription) {
      client->template send_request<QueryOrderSubmissionsService>(
        Beam::make_real_time_query(subscription));
    });
    m_order_submission_publisher.recover(*client);
    m_execution_report_publisher.recover(*client);
  }

  template<typename B>
  void ServiceOrderExecutionClient<B>::recover_orders(
      ServiceProtocolClient& client) {
    auto entries =
      std::unordered_map<Beam::DirectoryEntry, std::vector<OrderId>>();
    m_orders.for_each_value([&] (const auto& order) {
      order->with([&] (auto status, const auto& reports) {
        if(!is_terminal(status)) {
          entries[order->get_info().m_fields.m_account].push_back(
            order->get_info().m_id);
        }
      });
    });
    for(auto& entry : entries) {
      auto query = AccountQuery();
      query.set_index(entry.first);
      query.set_range(Beam::Sequence::FIRST, Beam::Sequence::PRESENT);
      query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
      auto id_expressions = std::vector<Beam::Expression>();
      for(auto& id : entry.second) {
        id_expressions.push_back(
          OrderInfoAccessor::from_parameter(0).get_order_id() == id);
      }
      query.set_filter(
        Beam::disjunction(id_expressions.begin(), id_expressions.end()));
      auto result =
        client.template send_request<QueryOrderSubmissionsService>(query);
      for(auto& record : result.m_snapshot) {
        auto order = m_orders.get(record->m_info.m_id);
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
    if(auto order = m_orders.try_load(report.m_id)) {
      (*order)->update(report);
    } else {
      m_execution_report_log.with([&] (auto& log) {
        auto i = std::lower_bound(log.begin(), log.end(), report,
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
