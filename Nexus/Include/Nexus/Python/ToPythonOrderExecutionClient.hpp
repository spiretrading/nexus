#ifndef NEXUS_TO_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_TO_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"

namespace Nexus::OrderExecutionService {

  /** Wraps an OrderExecutionClient for use with Python.
      \tparam ClientType The type of OrderExecutionClient to wrap.
   */
  template<typename ClientType>
  class ToPythonOrderExecutionClient final :
      public VirtualOrderExecutionClient {
    public:

      //! The type of OrderExecutionClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonOrderExecutionClient.
      /*!
        \param client The OrderExecutionClient to wrap.
      */
      ToPythonOrderExecutionClient(std::unique_ptr<Client> client);

      ~ToPythonOrderExecutionClient() override;

      void QueryOrderRecords(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue) override;

      void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue)
        override;

      void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue) override;

      void QueryExecutionReports(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue)
        override;

      const OrderExecutionPublisher& GetOrderSubmissionPublisher() override;

      const Order& Submit(const OrderFields& fields) override;

      void Cancel(const Order& order) override;

      void Update(OrderId orderId,
        const ExecutionReport& executionReport) override;

      void Open() override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  //! Makes a ToPythonOrderExecutionClient.
  /*!
    \param client The OrderExecutionClient to wrap.
  */
  template<typename Client>
  auto MakeToPythonOrderExecutionClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonOrderExecutionClient<Client>>(
      std::move(client));
  }

  template<typename ClientType>
  ToPythonOrderExecutionClient<ClientType>::ToPythonOrderExecutionClient(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonOrderExecutionClient<ClientType>::~ToPythonOrderExecutionClient() {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    Close();
    m_client.reset();
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::QueryOrderRecords(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue) {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    m_client->QueryOrderRecords(query, queue);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::QueryOrderSubmissions(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue) {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    m_client->QueryOrderSubmissions(query, queue);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::QueryOrderSubmissions(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue) {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    m_client->QueryOrderSubmissions(query, queue);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::QueryExecutionReports(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue) {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    m_client->QueryExecutionReports(query, queue);
  }

  template<typename ClientType>
  const OrderExecutionPublisher& ToPythonOrderExecutionClient<
      ClientType>::GetOrderSubmissionPublisher() {
    return m_client->GetOrderSubmissionPublisher();
  }

  template<typename ClientType>
  const Order& ToPythonOrderExecutionClient<ClientType>::Submit(
      const OrderFields& fields) {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    return m_client->Submit(fields);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::Cancel(const Order& order) {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    m_client->Cancel(order);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::Update(OrderId orderId,
      const ExecutionReport& executionReport) {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    m_client->Update(orderId, executionReport);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::Open() {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::Close() {
    auto gil = Beam::Python::GilRelease();
    auto release = boost::lock_guard(gil);
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::Shutdown() {
    m_client->Close();
    m_openState.SetClosed();
  }
}

#endif
