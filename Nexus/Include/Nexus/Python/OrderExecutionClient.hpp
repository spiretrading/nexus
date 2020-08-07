#ifndef NEXUS_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#include <Beam/Python/GilRelease.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Wraps an OrderExecutionClient for use with Python.
   * @param <C> The type of OrderExecutionClient to wrap.
   */
  template<typename C>
  class ToPythonOrderExecutionClient final :
      public VirtualOrderExecutionClient {
    public:

      /** The type of OrderExecutionClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonOrderExecutionClient.
       * @param client The OrderExecutionClient to wrap.
       */
      ToPythonOrderExecutionClient(std::unique_ptr<Client> client);

      ~ToPythonOrderExecutionClient() override;

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
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonOrderExecutionClient.
   * @param client The OrderExecutionClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonOrderExecutionClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonOrderExecutionClient<Client>>(
      std::move(client));
  }

  template<typename C>
  ToPythonOrderExecutionClient<C>::ToPythonOrderExecutionClient(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonOrderExecutionClient<C>::~ToPythonOrderExecutionClient() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  void ToPythonOrderExecutionClient<C>::QueryOrderRecords(
      const AccountQuery& query, Beam::ScopedQueueWriter<OrderRecord> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryOrderRecords(query, std::move(queue));
  }

  template<typename C>
  void ToPythonOrderExecutionClient<C>::QueryOrderSubmissions(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryOrderSubmissions(query, std::move(queue));
  }

  template<typename C>
  void ToPythonOrderExecutionClient<C>::QueryOrderSubmissions(
      const AccountQuery& query, Beam::ScopedQueueWriter<const Order*> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryOrderSubmissions(query, std::move(queue));
  }

  template<typename C>
  void ToPythonOrderExecutionClient<C>::QueryExecutionReports(
      const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryExecutionReports(query, std::move(queue));
  }

  template<typename C>
  const Order& ToPythonOrderExecutionClient<C>::Submit(
      const OrderFields& fields) {
    auto release = Beam::Python::GilRelease();
    return m_client->Submit(fields);
  }

  template<typename C>
  void ToPythonOrderExecutionClient<C>::Cancel(const Order& order) {
    auto release = Beam::Python::GilRelease();
    m_client->Cancel(order);
  }

  template<typename C>
  void ToPythonOrderExecutionClient<C>::Update(OrderId orderId,
      const ExecutionReport& executionReport) {
    auto release = Beam::Python::GilRelease();
    m_client->Update(orderId, executionReport);
  }

  template<typename C>
  void ToPythonOrderExecutionClient<C>::Open() {
    auto release = Beam::Python::GilRelease();
    m_client->Open();
  }

  template<typename C>
  void ToPythonOrderExecutionClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
