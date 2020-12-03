#ifndef NEXUS_ORDER_EXECUTION_CLIENT_BOX_HPP
#define NEXUS_ORDER_EXECUTION_CLIENT_BOX_HPP
#include <memory>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Provides a generic interface over an arbitrary OrderExecutionClient. */
  class OrderExecutionClientBox {
    public:

      /**
       * Constructs a OrderExecutionClientBox of a specified type using
       * emplacement.
       * @param <T> The type of order execution client to emplace.
       * @param args The arguments to pass to the emplaced order execution
       *        client.
       */
      template<typename T, typename... Args>
      explicit OrderExecutionClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a OrderExecutionClientBox by copying an existing order
       * execution client.
       * @param client The client to copy.
       */
      template<typename OrderExecutionClient>
      explicit OrderExecutionClientBox(OrderExecutionClient client);

      explicit OrderExecutionClientBox(OrderExecutionClientBox* client);

      explicit OrderExecutionClientBox(
        const std::shared_ptr<OrderExecutionClientBox>& client);

      explicit OrderExecutionClientBox(
        const std::unique_ptr<OrderExecutionClientBox>& client);

      void QueryOrderRecords(const AccountQuery& query,
        Beam::ScopedQueueWriter<OrderRecord> queue);

      void QueryOrderSubmissions(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrder> queue);

      void QueryOrderSubmissions(const AccountQuery& query,
        Beam::ScopedQueueWriter<const Order*> queue);

      void QueryExecutionReports(const AccountQuery& query,
        Beam::ScopedQueueWriter<ExecutionReport> queue);

      const Order& Submit(const OrderFields& fields);

      void Cancel(const Order& order);

      void Update(OrderId orderId, const ExecutionReport& executionReport);

      void Close();

    private:
      struct VirtualOrderExecutionClient {
        virtual ~VirtualOrderExecutionClient() = default;
        virtual void QueryOrderRecords(const AccountQuery& query,
          Beam::ScopedQueueWriter<OrderRecord> queue) = 0;
        virtual void QueryOrderSubmissions(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedOrder> queue) = 0;
        virtual void QueryOrderSubmissions(const AccountQuery& query,
          Beam::ScopedQueueWriter<const Order*> queue) = 0;
        virtual void QueryExecutionReports(const AccountQuery& query,
          Beam::ScopedQueueWriter<ExecutionReport> queue) = 0;
        virtual const Order& Submit(const OrderFields& fields) = 0;
        virtual void Cancel(const Order& order) = 0;
        virtual void Update(OrderId orderId,
          const ExecutionReport& executionReport) = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedOrderExecutionClient final : VirtualOrderExecutionClient {
        using OrderExecutionClient = C;
        Beam::GetOptionalLocalPtr<OrderExecutionClient> m_client;

        template<typename... Args>
        WrappedOrderExecutionClient(Args&&... args);
        void QueryOrderRecords(const AccountQuery& query,
          Beam::ScopedQueueWriter<OrderRecord> queue) override;
        void QueryOrderSubmissions(const AccountQuery& query,
          Beam::ScopedQueueWriter<SequencedOrder> queue) override;
        void QueryOrderSubmissions(const AccountQuery& query,
          Beam::ScopedQueueWriter<const Order*> queue) override;
        void QueryExecutionReports(const AccountQuery& query,
          Beam::ScopedQueueWriter<ExecutionReport> queue) override;
        const Order& Submit(const OrderFields& fields) override;
        void Cancel(const Order& order) override;
        void Update(OrderId orderId,
          const ExecutionReport& executionReport) override;
        void Close() override;
      };
      std::shared_ptr<VirtualOrderExecutionClient> m_client;
  };

  template<typename T, typename... Args>
  OrderExecutionClientBox::OrderExecutionClientBox(std::in_place_type_t<T>,
    Args&&... args)
    : m_client(std::make_shared<WrappedOrderExecutionClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename OrderExecutionClient>
  OrderExecutionClientBox::OrderExecutionClientBox(OrderExecutionClient client)
    : OrderExecutionClientBox(std::in_place_type<OrderExecutionClient>,
        std::move(client)) {}

  inline OrderExecutionClientBox::OrderExecutionClientBox(
    OrderExecutionClientBox* client)
    : OrderExecutionClientBox(*client) {}

  inline OrderExecutionClientBox::OrderExecutionClientBox(
    const std::shared_ptr<OrderExecutionClientBox>& client)
    : OrderExecutionClientBox(*client) {}

  inline OrderExecutionClientBox::OrderExecutionClientBox(
    const std::unique_ptr<OrderExecutionClientBox>& client)
    : OrderExecutionClientBox(*client) {}

  inline void OrderExecutionClientBox::QueryOrderRecords(
      const AccountQuery& query, Beam::ScopedQueueWriter<OrderRecord> queue) {
    m_client->QueryOrderRecords(query, std::move(queue));
  }

  inline void OrderExecutionClientBox::QueryOrderSubmissions(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    m_client->QueryOrderSubmissions(query, std::move(queue));
  }

  inline void OrderExecutionClientBox::QueryOrderSubmissions(
      const AccountQuery& query, Beam::ScopedQueueWriter<const Order*> queue) {
    m_client->QueryOrderSubmissions(query, std::move(queue));
  }

  inline void OrderExecutionClientBox::QueryExecutionReports(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    m_client->QueryExecutionReports(query, std::move(queue));
  }

  inline const Order& OrderExecutionClientBox::Submit(
      const OrderFields& fields) {
    return m_client->Submit(fields);
  }

  inline void OrderExecutionClientBox::Cancel(const Order& order) {
    m_client->Cancel(order);
  }

  inline void OrderExecutionClientBox::Update(OrderId orderId,
      const ExecutionReport& executionReport) {
    m_client->Update(orderId, executionReport);
  }

  inline void OrderExecutionClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  OrderExecutionClientBox::WrappedOrderExecutionClient<C>::
    WrappedOrderExecutionClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void OrderExecutionClientBox::WrappedOrderExecutionClient<C>::
      QueryOrderRecords(const AccountQuery& query,
        Beam::ScopedQueueWriter<OrderRecord> queue) {
    m_client->QueryOrderRecords(query, std::move(queue));
  }

  template<typename C>
  void OrderExecutionClientBox::WrappedOrderExecutionClient<C>::
      QueryOrderSubmissions(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrder> queue) {
    m_client->QueryOrderSubmissions(query, std::move(queue));
  }

  template<typename C>
  void OrderExecutionClientBox::WrappedOrderExecutionClient<C>::
      QueryOrderSubmissions(const AccountQuery& query,
        Beam::ScopedQueueWriter<const Order*> queue) {
    m_client->QueryOrderSubmissions(query, std::move(queue));
  }

  template<typename C>
  void OrderExecutionClientBox::WrappedOrderExecutionClient<C>::
      QueryExecutionReports(const AccountQuery& query,
        Beam::ScopedQueueWriter<ExecutionReport> queue) {
    m_client->QueryExecutionReports(query, std::move(queue));
  }

  template<typename C>
  const Order& OrderExecutionClientBox::WrappedOrderExecutionClient<C>::Submit(
      const OrderFields& fields) {
    return m_client->Submit(fields);
  }

  template<typename C>
  void OrderExecutionClientBox::WrappedOrderExecutionClient<C>::Cancel(
      const Order& order) {
    m_client->Cancel(order);
  }

  template<typename C>
  void OrderExecutionClientBox::WrappedOrderExecutionClient<C>::Update(
      OrderId orderId, const ExecutionReport& executionReport) {
    m_client->Update(orderId, executionReport);
  }

  template<typename C>
  void OrderExecutionClientBox::WrappedOrderExecutionClient<C>::Close() {
    m_client->Close();
  }
}

#endif
