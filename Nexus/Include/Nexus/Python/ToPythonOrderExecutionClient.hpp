#ifndef NEXUS_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"

namespace Nexus {

  /**
   * Wraps an OrderExecutionClient for use with Python.
   * @param <C> The type of OrderExecutionClient to wrap.
   */
  template<IsOrderExecutionClient C>
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
      const Client& get_client() const;

      /** Returns the wrapped client. */
      Client& get_client();

      std::shared_ptr<const Order> load_order(OrderId id);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderRecord> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<OrderRecord> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrder> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<std::shared_ptr<const Order>> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedExecutionReport> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<ExecutionReport> queue);
      std::shared_ptr<const Order> submit(const OrderFields& fields);
      void cancel(const std::shared_ptr<const Order>& order);
      void cancel(const Order& order);
      void update(OrderId id, const ExecutionReport& report);
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonOrderExecutionClient(
        const ToPythonOrderExecutionClient&) = delete;
      ToPythonOrderExecutionClient& operator =(
        const ToPythonOrderExecutionClient&) = delete;
  };

  template<typename Client>
  ToPythonOrderExecutionClient(Client&&) ->
    ToPythonOrderExecutionClient<std::remove_reference_t<Client>>;

  template<IsOrderExecutionClient C>
  template<typename... Args, typename>
  ToPythonOrderExecutionClient<C>::ToPythonOrderExecutionClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsOrderExecutionClient C>
  ToPythonOrderExecutionClient<C>::~ToPythonOrderExecutionClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsOrderExecutionClient C>
  const typename ToPythonOrderExecutionClient<C>::Client&
      ToPythonOrderExecutionClient<C>::get_client() const {
    return *m_client;
  }

  template<IsOrderExecutionClient C>
  typename ToPythonOrderExecutionClient<C>::Client&
      ToPythonOrderExecutionClient<C>::get_client() {
    return *m_client;
  }

  template<IsOrderExecutionClient C>
  std::shared_ptr<const Order>
      ToPythonOrderExecutionClient<C>::load_order(OrderId id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_order(id);
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderRecord> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<OrderRecord> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<std::shared_ptr<const Order>> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedExecutionReport> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsOrderExecutionClient C>
  std::shared_ptr<const Order>
      ToPythonOrderExecutionClient<C>::submit(const OrderFields& fields) {
    auto release = Beam::Python::GilRelease();
    return m_client->submit(fields);
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::cancel(
      const std::shared_ptr<const Order>& order) {
    auto release = Beam::Python::GilRelease();
    m_client->cancel(order);
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::cancel(const Order& order) {
    auto release = Beam::Python::GilRelease();
    m_client->cancel(order);
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::update(
      OrderId id, const ExecutionReport& report) {
    auto release = Beam::Python::GilRelease();
    m_client->update(id, report);
  }

  template<IsOrderExecutionClient C>
  void ToPythonOrderExecutionClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
