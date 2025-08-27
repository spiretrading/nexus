#ifndef NEXUS_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_ORDER_EXECUTION_CLIENT_HPP
#include <memory>
#include <utility>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"

namespace Nexus {

  /** Provides a generic interface over an arbitrary OrderExecutionClient. */
  class OrderExecutionClient {
    public:

      /**
       * Constructs an OrderExecutionClient of a specified type using
       * emplacement.
       * @param <T> The type of order execution client to emplace.
       * @param args The arguments to pass to the emplaced order execution
       *        client.
       */
      template<typename T, typename... Args>
      explicit OrderExecutionClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an OrderExecutionClientBox by copying an existing order
       * execution client.
       * @param client The client to copy.
       */
      template<typename C>
      explicit OrderExecutionClient(C client);

      explicit OrderExecutionClient(OrderExecutionClient* client);

      explicit OrderExecutionClient(
        const std::shared_ptr<OrderExecutionClient>& client);

      explicit OrderExecutionClient(
        const std::unique_ptr<OrderExecutionClient>& client);

      /** Loads an Order by its id. */
      std::shared_ptr<Order> load_order(OrderId id);

      /**
       * Submits a query for SequencedOrderRecords.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderRecord> queue);

      /**
       * Submits a query for OrderRecords.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<OrderRecord> queue);

      /**
       * Submits a query for SequencedOrder submissions.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrder> queue);

      /**
       * Submits a query for Order submissions.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue);

      /**
       * Submits a query for SequencedExecutionReports.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedExecutionReport> queue);

      /**
       * Submits a query for ExecutionReports.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<ExecutionReport> queue);

      /**
       * Submits a new single Order.
       * @param fields The OrderFields to submit.
       * @return The Order that was submitted.
       */
      std::shared_ptr<Order> submit(const OrderFields& fields);

      /**
       * Cancels an Order.
       * @param order The Order to cancel.
       */
      void cancel(const std::shared_ptr<Order>& order);

      /**
       * Cancels an Order.
       * @param order The Order to cancel.
       */
      void cancel(const Order& order);

      /**
       * Updates an Order.
       * @param id The id of the Order to update.
       * @param report The ExecutionReport containing the update.
       */
      void update(OrderId id, const ExecutionReport& report);

      void close();

    private:
      struct VirtualOrderExecutionClient {
        virtual ~VirtualOrderExecutionClient() = default;
        virtual std::shared_ptr<Order> load_order(OrderId id) = 0;
        virtual void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedOrderRecord> queue) = 0;
        virtual void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<OrderRecord> queue) = 0;
        virtual void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedOrder> queue) = 0;
        virtual void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) = 0;
        virtual void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedExecutionReport> queue) = 0;
        virtual void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<ExecutionReport> queue) = 0;
        virtual std::shared_ptr<Order> submit(const OrderFields& fields) = 0;
        virtual void cancel(const Order& order) = 0;
        virtual void update(OrderId id, const ExecutionReport& report) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedOrderExecutionClient final : VirtualOrderExecutionClient {
        using OrderExecutionClient = C;
        Beam::GetOptionalLocalPtr<OrderExecutionClient> m_client;

        template<typename... Args>
        WrappedOrderExecutionClient(Args&&... args);
        std::shared_ptr<Order> load_order(OrderId id) override;
        void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedOrderRecord> queue) override;
        void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<OrderRecord> queue) override;
        void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedOrder> queue) override;
        void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) override;
        void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedExecutionReport> queue) override;
        void query(const AccountQuery& query,
          Beam::ScopedQueueWriter<ExecutionReport> queue) override;
        std::shared_ptr<Order> submit(const OrderFields& fields) override;
        void cancel(const Order& order) override;
        void update(OrderId id, const ExecutionReport& report) override;
        void close() override;
      };
      std::shared_ptr<VirtualOrderExecutionClient> m_client;
  };

  /** Checks if a type implements an OrderExecutionClient. */
  template<typename T>
  concept IsOrderExecutionClient = std::constructible_from<
    OrderExecutionClient, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  OrderExecutionClient::OrderExecutionClient(
    std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedOrderExecutionClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename C>
  OrderExecutionClient::OrderExecutionClient(C client)
    : OrderExecutionClient(std::in_place_type<C>, std::move(client)) {}

  inline OrderExecutionClient::OrderExecutionClient(
    OrderExecutionClient* client)
    : OrderExecutionClient(*client) {}

  inline OrderExecutionClient::OrderExecutionClient(
    const std::shared_ptr<OrderExecutionClient>& client)
    : OrderExecutionClient(*client) {}

  inline OrderExecutionClient::OrderExecutionClient(
    const std::unique_ptr<OrderExecutionClient>& client)
    : OrderExecutionClient(*client) {}

  inline std::shared_ptr<Order> OrderExecutionClient::load_order(OrderId id) {
    return m_client->load_order(id);
  }

  inline void OrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderRecord> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void OrderExecutionClient::query(
      const AccountQuery& query, Beam::ScopedQueueWriter<OrderRecord> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void OrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void OrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void OrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedExecutionReport> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void OrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    m_client->query(query, std::move(queue));
  }

  inline std::shared_ptr<Order>
      OrderExecutionClient::submit(const OrderFields& fields) {
    return m_client->submit(fields);
  }

  inline void OrderExecutionClient::cancel(
      const std::shared_ptr<Order>& order) {
    m_client->cancel(*order);
  }

  inline void OrderExecutionClient::cancel(const Order& order) {
    m_client->cancel(order);
  }

  inline void OrderExecutionClient::update(
      OrderId id, const ExecutionReport& report) {
    m_client->update(id, report);
  }

  inline void OrderExecutionClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  OrderExecutionClient::WrappedOrderExecutionClient<C>::
    WrappedOrderExecutionClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::shared_ptr<Order>
      OrderExecutionClient::WrappedOrderExecutionClient<C>::load_order(
        OrderId id) {
    return m_client->load_order(id);
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::query(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderRecord> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::query(
      const AccountQuery& query, Beam::ScopedQueueWriter<OrderRecord> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::query(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::query(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::query(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedExecutionReport> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::query(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  std::shared_ptr<Order>
      OrderExecutionClient::WrappedOrderExecutionClient<C>::submit(
        const OrderFields& fields) {
    return m_client->submit(fields);
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::cancel(
      const Order& order) {
    m_client->cancel(order);
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::update(
      OrderId id, const ExecutionReport& report) {
    m_client->update(id, report);
  }

  template<typename C>
  void OrderExecutionClient::WrappedOrderExecutionClient<C>::close() {
    m_client->close();
  }
}

#endif
