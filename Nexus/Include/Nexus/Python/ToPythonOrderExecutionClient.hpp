#ifndef NEXUS_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/OrderExecutionService/OrderExecutionClientBox.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Wraps an OrderExecutionClient for use with Python.
   * @param <C> The type of OrderExecutionClient to wrap.
   */
  template<typename C>
  class ToPythonOrderExecutionClient {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonOrderExecutionClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonOrderExecutionClient, Args...>>
      ToPythonOrderExecutionClient(Args&&... args);

      ~ToPythonOrderExecutionClient();

      /** Returns the wrapped client. */
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

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

      void Update(OrderId orderId,
        const ExecutionReport& executionReport);

      void Close();

    private:
      boost::optional<Client> m_client;

      ToPythonOrderExecutionClient(
        const ToPythonOrderExecutionClient&) = delete;
      ToPythonOrderExecutionClient& operator =(
        const ToPythonOrderExecutionClient&) = delete;
  };

  template<typename Client>
  ToPythonOrderExecutionClient(Client&&) ->
    ToPythonOrderExecutionClient<std::decay_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonOrderExecutionClient<C>::ToPythonOrderExecutionClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonOrderExecutionClient<C>::~ToPythonOrderExecutionClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonOrderExecutionClient<C>::Client&
      ToPythonOrderExecutionClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonOrderExecutionClient<C>::Client&
      ToPythonOrderExecutionClient<C>::GetClient() {
    return *m_client;
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
  void ToPythonOrderExecutionClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
