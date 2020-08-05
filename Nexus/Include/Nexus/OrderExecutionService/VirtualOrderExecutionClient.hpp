#ifndef NEXUS_VIRTUAL_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_VIRTUAL_ORDER_EXECUTION_CLIENT_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Provides a pure virtual interface to an OrderExecutionClient. */
  class VirtualOrderExecutionClient : private boost::noncopyable {
    public:
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

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualOrderExecutionClient.
      VirtualOrderExecutionClient() = default;
  };

  /** Wraps an OrderExecutionClient providing it with a virtual interface.
      \tparam ClientType The type of OrderExecutionClient to wrap.
   */
  template<typename ClientType>
  class WrapperOrderExecutionClient : public VirtualOrderExecutionClient {
    public:

      //! The OrderExecutionClient to wrap.
      using Client = Beam::GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperOrderExecutionClient.
      /*!
        \param client The OrderExecutionClient to wrap.
      */
      template<typename OrderExecutionClientForward>
      explicit WrapperOrderExecutionClient(
        OrderExecutionClientForward&& client);

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

      void Open() override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<ClientType> m_client;
  };

  //! Wraps an OrderExecutionClient into a VirtualOrderExecutionClient.
  /*!
    \param client The client to wrap.
  */
  template<typename OrderExecutionClient>
  std::unique_ptr<VirtualOrderExecutionClient> MakeVirtualOrderExecutionClient(
      OrderExecutionClient&& client) {
    return std::make_unique<WrapperOrderExecutionClient<OrderExecutionClient>>(
      std::forward<OrderExecutionClient>(client));
  }

  template<typename ClientType>
  template<typename OrderExecutionClientForward>
  WrapperOrderExecutionClient<ClientType>::WrapperOrderExecutionClient(
      OrderExecutionClientForward&& client)
      : m_client(std::forward<OrderExecutionClientForward>(client)) {}

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::QueryOrderRecords(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<OrderRecord> queue) {
    m_client->QueryOrderRecords(query, std::move(queue));
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::QueryOrderSubmissions(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    m_client->QueryOrderSubmissions(query, std::move(queue));
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::QueryOrderSubmissions(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<const Order*> queue) {
    m_client->QueryOrderSubmissions(query, std::move(queue));
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::QueryExecutionReports(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    m_client->QueryExecutionReports(query, std::move(queue));
  }

  template<typename ClientType>
  const Order& WrapperOrderExecutionClient<ClientType>::Submit(
      const OrderFields& fields) {
    return m_client->Submit(fields);
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::Cancel(const Order& order) {
    m_client->Cancel(order);
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::Update(OrderId orderId,
      const ExecutionReport& executionReport) {
    m_client->Update(orderId, executionReport);
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::Close() {
    m_client->Close();
  }
}

#endif
